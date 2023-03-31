/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "ScElabDatabase.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/expr/ScParseExprValue.h"
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/AST/Type.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Decl.h>

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace sc;
using std::cout; using std::endl;

namespace sc_elab {


//  ScElabDatabase -------------------------------------------------------------

// Create @parseValue with @isCombProcess = true
ElabDatabase::ElabDatabase(sc_elab::SCDesign &scDesign,
                           sc_elab::ElabTypeManager &typeManager,
                           clang::ASTContext &astCtx)
    : designDB(scDesign), typeManager(typeManager), astCtx(astCtx),
      parseValue(astCtx, std::make_shared<ScState>(), true, NO_VALUE)
{

    std::vector<RecordView> records;

    for (const sc_elab::Object &obj : designDB.objects()) {
        ObjectView objView(&obj, this);

        if (auto record = objView.record()) {
            records.push_back(*record);
        }
    }
    
    for (auto &record : records) {
        // static data members are not stored in elaborator state
        // so we need to grab them from clang AST
        auto* cxxRecordDecl = record.getType().getTypePtr()->getAsCXXRecordDecl();

        bool isChannelRecord = false;
        if (!record.isTopMod()) {
            auto parent =  record.getParent();
            isChannelRecord = isScChannel(parent.getType(), true);

//            cout << "-------- " << cxxRecordDecl->getDeclName().getAsString() 
//                 << " " << (record.getFieldName() ? *record.getFieldName() : "")
//                 << " parent " << (parent.getFieldName() ? *parent.getFieldName() : "") 
//                 << " isChannelRecord " << isChannelRecord << endl;
        }
        
        // Skip static constant fields in channel record 
        if (!isChannelRecord) {
            for (clang::Decl* decl: cxxRecordDecl->decls()) {
                if (clang::VarDecl *varDecl = llvm::dyn_cast<clang::VarDecl>(decl)) {
                    if (varDecl->isStaticDataMember()) {
                        createStaticVariable(record, varDecl);
                    }
                }
            }
        }
    }
}

ObjectView ElabDatabase::createStaticVariable(RecordView parent, 
                                              const VarDecl* varDecl) 
{
    using std::cout; using std::endl;
    
    QualType varType = varDecl->getType();
    sc_elab::Object* newObj = createStaticObject(varType, parent.getID());
    //cout << "   createStaticVariable " << varDecl->getName().str() 
    //     << " parent " << (parent.getFieldName() ? *parent.getFieldName() : "") << endl;
    
    newObj->set_field_name(varDecl->getName().str());

    sc_elab::Object* parentObj = designDB.mutable_objects(parent.getID());
    parentObj->mutable_record()->add_member_ids(newObj->id());

    // Report all unsupported types 
    if (isScNotSupported(varType, true)) {
        ScDiag::reportScDiag(varDecl->getBeginLoc(),
                             ScDiag::SYNTH_TYPE_NOT_SUPPORTED) 
                            << varType.getAsString();
    }
    
    if (isUserClass(getDerefType(varType))) {
        SCT_INTERNAL_FATAL (varDecl->getBeginLoc(), 
                            "Static record is not supported yet");
    } else 
    if (isArray(varType)) {
        if (isStdVector(varType)) {
            ScDiag::reportScDiag(varDecl->getBeginLoc(), 
                                 ScDiag::CPP_STATIC_STD_VECTOR);
        }
        
        size_t arraySize = getArraySize(varType);
        SCT_TOOL_ASSERT(arraySize, "No size extracted for static array");
        QualType elmType = getArrayDirectElementType(varType).getCanonicalType();

        newObj->set_kind(sc_elab::Object::ARRAY);
        newObj->mutable_array()->add_dims(arraySize);

        Expr* initExpr = const_cast<Expr*>(varDecl->getAnyInitializer());
        initExpr = removeExprCleanups(initExpr);
        
        clang::Expr::EvalResult evalResult;
        bool evaluated = initExpr->EvaluateAsRValue(evalResult, astCtx);

        if (evaluated) {
            initStaticArray(newObj, elmType, evalResult.Val);
            
        } else {
            if (auto initListExpr = dyn_cast<InitListExpr>(initExpr)) {
                std::vector<APSInt> intVals;

                for (unsigned i = 0; i < initListExpr->getNumInits(); ++i) {
                    auto expr = initListExpr->getInit(i);
                    SValue val = parseValue.evaluateConstInt(expr, false).second;

                    if (val.isInteger()) {
                        intVals.push_back(val.getInteger());
                    } else {
                        SCT_INTERNAL_FATAL (varDecl->getBeginLoc(), 
                            "Can not get integer for static array initializer");
                    }
                }
                
                initStaticArray(newObj, elmType, intVals);
                
            } else {
                SCT_INTERNAL_FATAL (varDecl->getBeginLoc(), 
                                    "Unsupported static array initializer");
            }
        }
        
    } else {
        // Single variable
        APSInt intVal(64);
        if (Expr* initExpr = const_cast<Expr*>(varDecl->getAnyInitializer())) {
            initExpr = removeExprCleanups(initExpr);

            // Replace CXXConstructExpr with its argument, required for SC data types
            if (auto ctorExpr = dyn_cast<CXXConstructExpr>(initExpr)) {
                if (ctorExpr->getNumArgs() == 1) {
                    initExpr = ctorExpr->getArg(0);
                }
            }

            clang::Expr::EvalResult evalResult;
            bool evaluated = initExpr->EvaluateAsRValue(evalResult, astCtx);

            if (evaluated) {
                SCT_TOOL_ASSERT (evalResult.Val.isInt(), 
                                 "No global static constant integer result");
                intVal = evalResult.Val.getInt();

            } else {
                SValue val = parseValue.evaluateConstInt(initExpr, false).second;

                if (val.isInteger()) {
                    intVal = val.getInteger();
                } else {
                    SCT_INTERNAL_FATAL (varDecl->getBeginLoc(), 
                        "Can not get integer for static constant initializer");
                }
            }
        } else {
            // This case when constexpr is not used, value does not matter
        }

        // Adjust integer primitive
        newObj->set_kind(sc_elab::Object::PRIMITIVE);
        sc_elab::Primitive* prim = newObj->mutable_primitive();
        prim->set_kind(sc_elab::Primitive::VALUE);

        sc_elab::InitialValue* initVal = prim->mutable_init_val();
        initVal->set_bitwidth(intVal.getBitWidth());
        initVal->set_dyn_bitwidth(false);

        if (intVal.isSigned())
            initVal->set_int64_value(intVal.getSExtValue());
        else
            initVal->set_uint64_value(intVal.getZExtValue());
    }

    return ObjectView(newObj, this);
}


void ElabDatabase::initStaticArray(sc_elab::Object* arrayObj, QualType elmType,
                                   clang::APValue initVals) 
{
    // Cope with std::array initializer, get array from record field
    if (initVals.isStruct()) {
        SCT_TOOL_ASSERT (initVals.getStructNumFields(), "No fields in init struct");
        initVals = initVals.getStructField(0);
    }
    
    if (!initVals.isArray()) {
        SCT_INTERNAL_ERROR_NOLOC("Incorrect initializer for static array");
        return;
    }

    std::size_t arraySize = arrayObj->array().dims(0);
    
    for (size_t idx = 0; idx < arraySize; ++idx) {
        // Skip extra elements which has no initializers
        if (idx >= initVals.getArrayInitializedElts()) continue;
        auto arrayInit = initVals.getArrayInitializedElt(idx);

        if (isArray(elmType)) {
            size_t arraySize = getArraySize(elmType);
            SCT_TOOL_ASSERT(arraySize, "No size extracted for static array");
            QualType childType = getArrayDirectElementType(elmType).getCanonicalType(); 

            auto elArrayObj = createStaticArrray(elmType, arrayObj->id(),
                                                 arraySize);
            
            arrayObj->mutable_array()->add_element_ids(elArrayObj->id());
            initStaticArray(elArrayObj, childType, arrayInit);

        } else {
            SCT_TOOL_ASSERT (elmType->isBuiltinType(), "No builtin type");
            auto *builtinType = dyn_cast<BuiltinType>(elmType);
            SCT_TOOL_ASSERT (builtinType->isInteger(), "Builtin type is not integer");
            SCT_TOOL_ASSERT (arrayInit.isInt(), "Builtin type value is not integer");

            auto newObj = createStaticPrimitive(elmType, arrayObj->id(),
                                                arrayInit.getInt());

            newObj->set_rel_type(sc_elab::Object::ARRAY_ELEMENT);
            newObj->set_array_idx(idx);
            arrayObj->mutable_array()->add_element_ids(newObj->id());
        }
    }
}

void ElabDatabase::initStaticArray(sc_elab::Object* arrayObj,
                                   QualType elementType,
                                   std::vector<APSInt>& initVals) 
{
    unsigned idx = 0;
    for (auto& init : initVals) 
    {
        if (elementType->isArrayType()) {
            SCT_INTERNAL_ERROR_NOLOC("Multi-dimensional static arrays not supported yet");

        } else {
            auto* newObj = createStaticPrimitive(elementType, arrayObj->id(), init);
     
            newObj->set_rel_type(sc_elab::Object::ARRAY_ELEMENT);
            newObj->set_array_idx(idx);
            arrayObj->mutable_array()->add_element_ids(newObj->id());
        }
        idx++;
    }
}

sc_elab::Object *ElabDatabase::createStaticPrimitive(clang::QualType varType,
                                                     uint32_t parentID,
                                                     llvm::APSInt &intInit) {
    sc_elab::Object * newObj = createStaticObject(varType, parentID);

    newObj->set_kind(sc_elab::Object::PRIMITIVE);
    sc_elab::Primitive * prim = newObj->mutable_primitive();

    prim->set_kind(sc_elab::Primitive::VALUE);
    sc_elab::InitialValue *initVal = prim->mutable_init_val();
    initVal->set_bitwidth(intInit.getBitWidth());
    initVal->set_dyn_bitwidth(false);

    if (intInit.isSigned())
        initVal->set_int64_value(intInit.getSExtValue());
    else
        initVal->set_int64_value(intInit.getZExtValue());

    return newObj;
}

uint32_t ElabDatabase::getOrCreateTypeID(clang::QualType varType) {
    return typeManager.getOrCreateTypeID(varType);
}

sc_elab::Object *ElabDatabase::createStaticObject(clang::QualType type,
                                                  uint32_t parentID) {

    sc_elab::Object * newObj = designDB.add_objects();

    newObj->set_id(designDB.objects_size() - 1);
    newObj->set_type_id(getOrCreateTypeID(type));
    newObj->set_is_constant(true);
    newObj->set_rel_type(sc_elab::Object::STATIC);
    newObj->set_sckind(sc_elab::Object::SC_NONE);
    newObj->add_parent_ids(parentID);

    designDB.mutable_objects(parentID)->mutable_record()->add_member_ids(newObj->id());

    return newObj;
}

sc_elab::Object *ElabDatabase::createStaticArrray(clang::QualType type,
                                                  uint32_t parentID,
                                                  uint32_t arraySize) {

    sc_elab::Object * newObj = createStaticObject(type, parentID);
    newObj->set_kind(sc_elab::Object::ARRAY);
    newObj->mutable_array()->add_dims(arraySize);

    return newObj;
}


ObjectView ElabDatabase::getObj(uint32_t objID) const
{
    return ObjectView(&designDB.objects(objID), this);
}

QualType ElabDatabase::getType(uint32_t typeID) const
{
    return typeManager.getTypeByID(typeID);
}

const std::vector<ModuleMIFView> &ElabDatabase::getModules() const
{
    if (modules.empty()) {
        for (auto modID : designDB.module_ids()) {
            if (auto moduleOrModIf = getObj(modID).moduleMIF())
                modules.push_back(*moduleOrModIf);
        }
    }
    return modules;
}

ModuleMIFView ElabDatabase::getTopModule() const
{
    return ObjectView(&designDB.objects(0), this);
}

const std::list<VerilogModule>& ElabDatabase::getVerilogModules() const
{
    return verilogMods;
}

std::list<VerilogModule> &ElabDatabase::getVerilogModules()
{
    return verilogMods;
}

VerilogModule *
ElabDatabase::getVerilogModule(ModuleMIFView scModView)
{
    return verModMap.at(scModView);
}

const VerilogModule *
ElabDatabase::getVerilogModule(ModuleMIFView scModView) const
{
    return verModMap.at(scModView);
}

bool ElabDatabase::hasVerilogModule(ModuleMIFView scModView) const
{
    return (verModMap.count(scModView) != 0);
}

VerilogModule * ElabDatabase::addVerilogModule(ModuleMIFView scModView)
{
    SCT_TOOL_ASSERT (!verModMap.count(scModView), "");
    verilogMods.emplace_back(scModView);
    verModMap[scModView] = &verilogMods.back();
    return &verilogMods.back();
}

void ElabDatabase::uniquifyVerilogModules()
{
    for (auto it = verilogMods.begin(); it != verilogMods.end(); ++it) {
        for (auto uit = std::next(it); uit != verilogMods.end(); ) {
            // Skip SVA property
            if (it->isEquivalentTo(*uit)) {
                verModMap[uit->getModObj()] = &(*it);
                // llvm::outs() << "SET " << uit->getModObj() << " == " << it->getModObj() <<"\n";
                uit = verilogMods.erase(uit);
            } else {
                ++uit;
            }
        }
    }
}

void ElabDatabase::dump() const
{
    llvm::outs() << "ElabDatabase DUMP\n";
    getTopModule().dumpHierarchy(true);
}

} // namespace sc_elab
