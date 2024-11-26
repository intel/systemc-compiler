/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "sc_tool/elab/ScElabProcBuilder.h"
#include "sc_tool/elab/ScElabDatabase.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/cthread/ScThreadBuilder.h"
#include "sc_tool/ScCommandLine.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/CommandLine.h>

#include <sstream>

using namespace sc;
using namespace llvm;
using namespace clang;
using namespace std;

namespace sc_elab {

ProcBuilder::ProcBuilder(ModuleMIFView moduleView, ElabDatabase &elabDB,
                         const std::unordered_set<uint32_t>& extrTargInit_)
    : ctx(*moduleView.getDatabase()->getASTContext())
    , rootModView(moduleView)
    , elabDB(elabDB)
    , extrTargInit(extrTargInit_)
{
    //cout << "ProcBuilder " << moduleView.getVerilogModule()->getName() << endl;
    // Module global state
    moduleState = std::make_shared<ScState>();
    
    // Fill state from elabDB, recursively add module fields
    traverseRecord(moduleView, true);
    SCT_TOOL_ASSERT (classHierStack.empty(), "Parent hierarchy is not empty");
    
    // Checking for NO_VALUE tuple in @state 
    moduleState->checkNoValueTuple();

    procAnalyzer = std::make_unique<ScProcAnalyzer>(ctx, elabDB, moduleState);
}

// Generate module level SVA properties code from declarations
std::string ProcBuilder::generateSvaProperties(VerilogModule& verMod) 
{
    SValue procHostClass = objSValMap.at(verMod.getModObj());
    if (!procHostClass.isRecord()) {
        procHostClass = moduleState->getValue(procHostClass);
    }
    //std::cout << "procHostClass " << procHostClass << std::endl;
    SCT_TOOL_ASSERT (procHostClass.isRecord(), "No record value for module");
    
    // Analyze temporal assertion and generate SVA string
    std::string svaPropStr = procAnalyzer->analyzeSvaProperties(
                             verMod, procHostClass, verMod.getSvaProperties());
    return svaPropStr;
}

// Run for each process in the module
VerilogProcCode ProcBuilder::generateVerilogProcess(ProcessView& procView)
{
    /// Host record and cxx Method for process
    std::pair<RecordView, clang::CXXMethodDecl*> procLoc = procView.getLocation();
    RecordView procRecordView = procLoc.first;

    ModuleMIFView hostModule = procView.getParentModuleOrMIF();

    SValue procHostClass = objSValMap.at(procRecordView);
    if (!procHostClass.isRecord()) {
        procHostClass = moduleState->getValue(procHostClass);
    }

    SValue hostModuleDynClass = objSValMap.at(hostModule);
    if (!hostModuleDynClass.isRecord()) {
        hostModuleDynClass = moduleState->getValue(hostModuleDynClass);
    }

    SCT_TOOL_ASSERT (procHostClass.isRecord(), "No record value for module");
    SCT_TOOL_ASSERT (hostModuleDynClass.isRecord(), "No record value for module");
    SCT_TOOL_ASSERT (classHierStack.empty(), "Parent hierarchy is not empty");

//    using std::cout; using std::endl;
//    cout << "procRecordView " << procRecordView.getDebugString() << " isMIF "
//         << procRecordView.isModularInterface() << procRecordView.isArrayElement() << endl;
//    cout << "procView " << procView.procName << " " << procView.isCombinational() << endl;

    if (procView.isCombinational()) {
        return procAnalyzer->analyzeMethodProcess(procHostClass,
                                hostModuleDynClass, procView);
    } else {
        return procAnalyzer->analyzeCthreadProcess(procHostClass,
                                hostModuleDynClass, procView);
    }

    return VerilogProcCode("");
}

void ProcBuilder::prepareState(ModuleMIFView hostModule)
{
    SValue hostModuleDynClass = objSValMap.at(hostModule);

    if (DebugOptions::isEnabled(DebugComponent::doElabState)) {
        moduleState->print();
    }
    moduleState->fillDerived();
    //moduleState->updateStaticClasses();   Not required
    
    if (DebugOptions::isEnabled(DebugComponent::doElabState)) {
        llvm::outs() << "INITIAL_STATE\n";
        moduleState->print();
        rootModView.dumpHierarchy(false);
    }
}

// \param isTop -- current module
sc::SValue ProcBuilder::traverseRecord(RecordView recView, bool isVerModule)
{
    using std::cout; using std::endl;
    DEBUG_WITH_TYPE(DebugOptions::doProcBuilder,
        outs() << "Traversing " << recView << "\n";
    );

    // If it exists just return it
    if (objSValMap.count(recView))
        return objSValMap.at(recView);

    std::vector<SValue> baseValues;

    // Traverse all bases classes
    for (RecordView baseClassView : recView.getBases())
        baseValues.push_back(traverseRecord(baseClassView));

    // Base class instantiated in its inheritor class but not class where
    // inheritor variable is declared
    // Any record except module instance have @parent
    SValue parent = classHierStack.empty() ? SValue() : classHierStack.back();

    SValue currentModSVal = RecordValues::getRecordView(recView);
    // No value for record not module/MIF, create value here as it cannot be
    // passed through pointer to another module
    if (currentModSVal.isUnknown()) {
        currentModSVal = SValue(recView.getType(), baseValues, parent);
    }
    classHierStack.push_back(currentModSVal);

    if (isVerModule) {
        // Variable for module
        SValue objectSVal(recView.getType(), ObjectOwner::ooTrue);
        moduleState->putValue(objectSVal, currentModSVal, false, false);
        objSValMap[recView] = objectSVal;
    }
    
//    cout << "   traverseField ... " << endl;
//    auto start = chrono::system_clock::now();
    
    // Check if this record is Target/Initiator in top module and bound to external
    bool isExtrTargInit = extrTargInit.count(recView.getID()) != 0;
    unsigned valOrigSync = 0; unsigned valChanSync = 0; 
    
    for (ObjectView memberObj : recView.getFields()) {
        // Normal traverse field for any record
        const SValue& lval = traverseField(memberObj);

        if (isExtrTargInit && lval) {
            if (lval.asString().find("orig_sync") != std::string::npos) {
                const SValue& rval = moduleState->getValue(lval);
                if (rval.isInteger()) valOrigSync = rval.getInteger().getZExtValue();
                //cout << lval << " " << rval << endl;
            }
            if (lval.asString().find("chan_sync") != std::string::npos) {
                const SValue& rval = moduleState->getValue(lval);
                if (rval.isInteger()) valChanSync = rval.getInteger().getZExtValue();
                //cout << lval << " " << rval << endl;
            }
        }
    }
    
    // Report error if sync value was updated in @bind() function
    if (valOrigSync != valChanSync) {
        if (recView.getValueDecl()) 
            ScDiag::reportScDiag(recView.getValueDecl()->getBeginLoc(), 
                                 ScDiag::SYNTH_SS_CHAN_PARAM_IN_TOP);
        else {
            if (auto pardecl = recView.getParentModuleOrMIF().getValueDecl()) {
                ScDiag::reportScDiag(pardecl->getBeginLoc(), 
                                     ScDiag::SYNTH_SS_CHAN_PARAM_IN_TOP);
            } else {
                ScDiag::reportScDiag(ScDiag::SYNTH_SS_CHAN_PARAM_IN_TOP);
            }
        }
    }
    
//    auto end = chrono::system_clock::now();
//    chrono::duration<double> diff = end-start;
//    cout << "   traverseField DONE, time " << diff.count() << endl;

    for (ObjectView staticObj : recView.getStaticFields()) {
        traverseField(staticObj);
    }

    //cout << "---------- unresolvedPointers " << endl;
    
    // try to find pointees if they were created during record traversal
    for (auto unresolvedPtr : unresolvedPointers) {
        //cout << "unresolvedPtr " << unresolvedPtr.getDebugString() << endl;
        llvm::Optional<ObjectView> pointee = unresolvedPtr.pointeeOrArray();

        if (pointee) {
            //cout << "pointee " << pointee->getDebugString() << endl;
            SValue pointerSVal = objSValMap.at(unresolvedPtr);
            
            if (objSValMap.count(*pointee)) {
                SValue pointeeSVal = objSValMap.at(*pointee);
                moduleState->putValue(pointerSVal, pointeeSVal, true, false);
                
            } else {
                // No pointee found, it can be object out of this module
                // Support only module/MIF object for now
                if (pointee->isModule() || pointee->isModularInterface()) {
                    SValue pointeeSVal = RecordValues::getRecordView(*pointee);
                    if (!pointeeSVal.isRecord()) {
                        cout << "Unresolved ptr " << unresolvedPtr.getDebugString()
                             << " pointee " << pointee->getDebugString()
                             << " pointeeSVal " << pointeeSVal << endl;
                        SCT_TOOL_ASSERT (false, "No record found");
                    }
                    moduleState->putValue(pointerSVal, pointeeSVal, true, false);
                    //cout << "Add value " << pointerSVal << " " << pointeeSVal << endl;
                }
            }
        }
    }
    unresolvedPointers.clear();
    
    // Replace pointed array variable to array object for pointers
    for (auto arrayPtr : arrayPointers) {
        const SValue& rval = moduleState->getValue(arrayPtr);
        const SValue& aval = moduleState->getValue(rval);
        if (aval.isArray()) {
            moduleState->putValue(arrayPtr, aval, false, false);
            //cout << "Replace " << rval << " to " << aval << endl;
        }
    }
    arrayPointers.clear();

    classHierStack.pop_back();

    objSValMap[recView] = currentModSVal;
    
    // Register modules and MIFs
    if (recView.isModule() || recView.isModularInterface()) {
        // Get base class for @sct_comb_target, it is @sct_target contains all members
        if ( isSctCombTarg(currentModSVal.getType()) ) {
            SCT_TOOL_ASSERT(!currentModSVal.getRecord().bases.empty(), 
                            "No base class for sct_comb_target found");
            SValue baseTargVal = currentModSVal.getRecord().bases.front();
            moduleState->putElabObject(baseTargVal, recView);
        } else {
            moduleState->putElabObject(currentModSVal, recView);
        }
    }
    
    return currentModSVal;
}

sc::SValue ProcBuilder::traverseField(ObjectView memberObj)
{
    DEBUG_WITH_TYPE(DebugOptions::doProcBuilder,
                    outs() << "Traversing field " << memberObj << "\n";
    );


    if (objSValMap.count(memberObj))
        return objSValMap.at(memberObj);

    clang::ValueDecl* valDecl = memberObj.getValueDecl();

    if (valDecl) {
        bool isStatic = memberObj.isStatic();

        SValue lSVal;
        if (isStatic) {
            lSVal = SValue(valDecl, SValue() );
        } else {
            lSVal = SValue(valDecl, classHierStack.back());
            // Pointer to constant flag to get value of its pointe
            constPointe = isPointerToConst(memberObj.getType());
        }

        SValue rSval = traverse(memberObj);
        constPointe = false;

        objSValMap[memberObj] = lSVal;

        // Register pointed array variable to replace it to array object
        if (lSVal.isPointer() && rSval.isVariable()) {
            if (sc::isArray(rSval.getType())) {
                arrayPointers.insert(lSVal);
                //cout << "-- arrayPointers lSVal " << lSVal << endl;
            }
        }
        
        moduleState->putValue(lSVal, rSval, false, false);
        moduleState->putElabObject(lSVal, memberObj);

        return lSVal;
    }

    return SValue();
}

// Cannot return SVariable 
sc::SValue ProcBuilder::traverse(ObjectView objView)
{
    // check if object already visited during pointer traversal
    if(objSValMap.count(objView))
        return objSValMap.at(objView);

    SValue res;

    if (objView.isPrimitive()) {
        res = traversePrimitive(objView);
    } else if (objView.isArrayLike()) {
        res = traverseArray(objView);
    } else if (objView.isSignal()){
        res = createSignalSValue(objView);
    } else if (objView.isRecord()) {
        // No module traverse here (not modular interface)
        if (!objView.isModule()) {
            res = traverseRecord(objView);
        }

    } else {
        reportUnsuported(objView);
    }

    if (objView.isDynamic())
        objSValMap[objView] = res;

    return res;
}

void ProcBuilder::reportUnsuported(ObjectView objView)
{
    if (auto fieldDeclOpt = objView.getValueDecl()) {
        auto *decl = fieldDeclOpt;
        ScDiag::reportScDiag(decl->getLocation(),
                             ScDiag::SC_WARN_ELAB_UNSUPPORTED_TYPE)
            << decl->getType().getAsString();
    } else {
        ScDiag::reportScDiag(ScDiag::SC_WARN_ELAB_UNSUPPORTED_TYPE)
            << objView.getType().getAsString();
    }
}


sc::SValue ProcBuilder::traversePrimitive(PrimitiveView primView)
{
    if (primView.isValue()) {
        return createPrimitiveSValue(primView);
    } else 
    if (primView.isPort()) {
        return createPortSValue(primView);
    } else 
    if (primView.isPointer() || primView.isReference()) {
        return getOrCreatePointeeSValue(primView);
    } else 
    if (primView.isString()) {
        // Strings not stored in state
        return SValue();
    } else {
        reportUnsuported(primView);
        return SValue();
    }
}

sc::SValue ProcBuilder::traverseArray(ArrayView arrayView)
{
    if (arrayView.hasElements()) {
        clang::QualType elemType = arrayView.at(0).getType();
        SValue arraySVal(elemType, arrayView.size(), 0);
        SValue arraySValElems = arraySVal;
                
        for (size_t i = 0; i < arrayView.size(); i++) {
            ObjectView elemObj = arrayView.at(i);
            SValue elemSVal = traverse(elemObj);
            arraySValElems.getArray().setOffset(i);
            
            // Only zero array element used to get channel name
            if (i == 0 || !elemSVal.isScChannel()) {
                moduleState->putValue(arraySValElems, elemSVal, false, false);
                moduleState->putElabObject(arraySValElems, elemObj); 
            }
            objSValMap[elemObj] = arraySValElems;
        }
        return arraySVal;
        
    } else {
        auto type = arrayView.getType();
        if (isStdArray(type)) {
            return moduleState->createStdArrayInState(type);
        } else {
            // Non-constant std:vector not-supported as its size cannot be evaluated 
            return moduleState->createArrayInState(type);
        }
    }
}


SValue ProcBuilder::createPrimitiveSValue(ValueView value)
{
    // Previously values are stored in state for constant or pointe of 
    // pointer to constant, now all member variables stored
    // Member variables modified in a process removed from initial state 
    //if (!value.isConstant() && !constPointe)
    //    return SValue();

    // Use decimal radix here, will be replaced in @ScGenerateExpr constructor
    if (auto uval = value.uint64Val()) {
        return SValue(APSInt(APInt(value.bitwidth(), *uval), true), 10);
    } else 
    if (auto ival = value.int64Val()) {
        return SValue(APSInt(APInt(value.bitwidth(), *ival), false), 10);
    } else {
        // Report error for biguint/bigint constants
        // TODO: remove me, #312
        if (value.isConstant() || constPointe) {
            ScDiag::reportScDiag(ScDiag::SC_ERROR_ELAB_UNSUPPORTED_TYPE)
                                 << value.getType();
        }

        return SValue();
        //~TODO
    }
}

sc::SValue ProcBuilder::createPortSValue(PortView portView)
{
    // @sc_port<if> is pointer to module/MIF
    if (!portView.isSignalPort()) {
        // Return pointer to module/MIF
        if (auto pointee = portView.pointee()) {
            SValue pointeeVal = RecordValues::getRecordView(*pointee);
            
            if (!pointeeVal.isRecord()) {
                cout << "Port " <<  portView.getDebugString()
                     << " pointee " << pointee->getDebugString()
                     << " pointeeSVal " << pointeeVal << endl;
                SCT_TOOL_ASSERT (false, "No record found");
            }
            return pointeeVal;
        }
        return SValue();
    }

    SValue res;
    auto verVars = portView.getVerilogVars();
    SignalView signalView(portView.getBindedSignal());
    auto sigValue  = signalView.getSignalValue();
    auto clangType = sigValue.getType();

    if (auto recView = sigValue.record()) {
        // Record port
        std::string verName = portView.getSCName()+std::string("_REC");
        
        if (portView.isInput())
            res = SValue(new ScInPort(verName, clangType));
        else if (portView.isOutput())
            res = SValue(new ScOutPort(verName, clangType));
        else {
            ScDiag::reportScDiag(ScDiag::SC_ERROR_ELAB_UNSUPPORTED_TYPE)
                << clangType->getCanonicalTypeInternal().getAsString();
            res = SValue();
        }
        
        unsigned i = 0;
        for (ObjectView fieldObj : recView->getFields()) {
            auto* fieldDecl = fieldObj.getValueDecl();
            SCT_TOOL_ASSERT(fieldDecl, "No declaration for channel record field");
            SCT_TOOL_ASSERT(!fieldObj.isStatic() || fieldObj.isConstant(), 
                            "Static non-constant field in channel record");

            // Skip zero width record field
            if (isZeroWidthType(fieldDecl->getType()) || 
                isZeroWidthArrayType(fieldDecl->getType())) continue;
            
            // Field with record signal as parent class
            SValue fval(fieldDecl, res);
            moduleState->putElabObject(fval, fieldObj, verVars[i].var);
            i++;
            //cout << "  fval " << fval << endl;
        }
    } else {
        // Normal port
        std::string verName;
        if (verVars.size() > 0)
            verName = verVars[0].var->getName();
        else {
            // Port of record not supported yet
            verName = "UNNAMED";
        }    

        if (portView.isInput())
            res = SValue(new ScInPort(verName, clangType));
        else if (portView.isOutput())
            res = SValue(new ScOutPort(verName, clangType));
        else {
            ScDiag::reportScDiag(ScDiag::SC_ERROR_ELAB_UNSUPPORTED_TYPE)
                << clangType->getCanonicalTypeInternal().getAsString();
            res = SValue();
        }
    }

    return res;
}

// Recursively gather all fields from the record view
void ProcBuilder::createSignalSValue(RecordView& recView, ElabObjVec& allFields) 
{
    for (RecordView& baseView : recView.getBases()) {
        createSignalSValue(baseView, allFields);
    }
    allFields.append(recView.getFields());
}

sc::SValue ProcBuilder::createSignalSValue(SignalView signalView)
{
    SValue res;
    auto verVars   = signalView.getVerilogVars();
    auto sigValue  = signalView.getSignalValue();
    auto clangType = sigValue.getType();
    //cout << "sigValue " << sigValue.getID() << " " << sigValue.getDebugString() << endl;
    
    if (auto recView = sigValue.record()) {
        // Record signal
        std::string verName = signalView.getName()+std::string("_REC");
        res = SValue(new ScSignal(verName, clangType));

        ElabObjVec allFields;
        createSignalSValue(*recView, allFields);
        
        unsigned i = 0;
        for (ObjectView fieldObj : allFields) {
            auto* fieldDecl = fieldObj.getValueDecl();
            SCT_TOOL_ASSERT(fieldDecl, "No declaration for channel record field");
            SCT_TOOL_ASSERT(!fieldObj.isStatic() || fieldObj.isConstant(), 
                            "Static non-constant field in channel record");
            // Skip zero width record field
            if (isZeroWidthType(fieldDecl->getType()) || 
                isZeroWidthArrayType(fieldDecl->getType())) continue;
            
            // Field with record signal as parent class
            SValue fval(fieldDecl, res);
            moduleState->putElabObject(fval, fieldObj, verVars[i].var);
            //cout << "  fval " << fval << " " << verVars[i].var->getName() << endl;
            i++;
        }
    } else {
        // Normal signal
        std::string verName;
        if (verVars.size() > 0) {
            verName = verVars[0].var->getName();
        } else {
            verName = "UNNAMED";
            SCT_INTERNAL_ERROR_NOLOC("No variable for signal " + signalView.getName());
        }
        res = SValue(new ScSignal(verName, clangType));
    }

    return res;
}

sc::SValue ProcBuilder::getOrCreatePointeeSValue(PtrOrRefView ptrOrRefView)
{
    if ( ptrOrRefView.isNotNullDangling())
        return NO_VALUE;

    if ( ptrOrRefView.isNull()) {
        return SValue(APSInt::getUnsigned(0), 10);
    }

    // If pointer is not initialized it can be eventually pointed to memory
    // with array-like data, so cannot detect that
    // Pointer to non-first element of non-constant integer array not supported
    if (ptrOrRefView.isBaseOffsetPtr() && (*ptrOrRefView.getOffset() != 0)
        && !ptrOrRefView.pointeeOrArray()->isConstant()) {

        if (auto ptrDecl = ptrOrRefView.getValueDecl()) {
            ScDiag::reportScDiag(ptrDecl->getBeginLoc(), 
                                 ScDiag::SC_ERROR_ELAB_BASE_OFFSET_PTR);
        } else {
            ScDiag::reportScDiag(ScDiag::SC_ERROR_ELAB_BASE_OFFSET_PTR);
        }

        return NO_VALUE;
    }

    // Get pointer value
    llvm::Optional<ObjectView> pointee = ptrOrRefView.pointeeOrArray();

    if (pointee) {
        if (objSValMap.count(*pointee))
            return objSValMap.at(*pointee);

        if (pointee->isDynamic()) {
            // Always create dynamic object as there is only one owner of it
            SValue rSval = traverse(*pointee);

            if (rSval.isArray()) {
                return rSval;
            } else {
                // Create SObject for non-array variable
                SValue objectSVal(pointee->getType(), ObjectOwner::ooTrue);
                moduleState->putValue(objectSVal, rSval, true, false);
                moduleState->putElabObject(objectSVal, *pointee);
                return objectSVal;
            }
        } else {
            // Delay the pointee object traverse as we do not know parent for it
            unresolvedPointers.push_back(ptrOrRefView);
        }
    }

    return NO_VALUE;
}

std::unordered_map<uint32_t, sc::SValue> ProcBuilder::recordMap;

//============================================================================

std::vector<SValue> RecordValues::getBaseValues(const RecordView& recView)
{
    std::vector<SValue> bases;
    for (RecordView& baseView : recView.getBases()) {
        // Parent is not important for base classes
        bases.push_back(getOrCreateRecordValue(baseView));
    }
    return bases;
}

sc::SValue RecordValues::getOrCreateRecordValue(const RecordView& recView) 
{
    //std::cout << "recView " << recView.getDebugString() << " id " << recView.getID() << std::endl;
    auto i = recordMap.find(recView.getID());

    if (i == recordMap.end()) {
        // Base class in not registered
        SValue val(recView.getType(), getBaseValues(recView), NO_VALUE);
        recordMap.emplace(recView.getID(), val);
        return val;
        
    } else 
    if (i->second.isUnknown()) {
        // Module is initial registered in @addRecordView() with NO_VALUE
        SValue val(recView.getType(), getBaseValues(recView), NO_VALUE);
        // Get tuple iterator again as @getBaseValues() updates @recordMap
        i = recordMap.find(recView.getID());
        i->second = val;
        return val;
        
    } else {
        return i->second;
    }
}

void RecordValues::createRecordValue(const RecordView& recView, 
                                     const SValue& parent) 
{
    SValue val(recView.getType(), getBaseValues(recView), parent);
    auto i = recordMap.find(recView.getID());
    i->second = val;
}


void RecordValues::fillTopModValue()
{
    for (const auto& i : recordMap) {
        if (i.second.isUnknown()) {
            const ObjectView& objView = elabDB->getObj(i.first);
            if (objView.record()->isTopMod()) {
                // No parent for top module
                createRecordValue(*objView.record());
                break;
            }
        }
    }
}

void RecordValues::fillValuesWithParent()
{
    bool done;
    do {
        done = true;
        for (const auto& i : recordMap) {
            if (i.second.isUnknown()) {
                const ObjectView& objView = elabDB->getObj(i.first);
                const auto& parView = objView.getParentModuleOrMIF();
                // Check for parent module is ready
                auto par = recordMap.find(parView.getID());
                if (par != recordMap.end() && !par->second.isUnknown()) {
                    createRecordValue(*objView.record(), par->second);
                    done = false; break;
                }
            }
        }
    } while (!done);
}

void RecordValues::addRecordView(const RecordView& recView) 
{
    //std::cout << "addRecordView ID " << recView.getID() << "  " << recView.getDebugString() << std::endl;
    recordMap.emplace(recView.getID(), NO_VALUE);
}

const SValue& RecordValues::getRecordView(const RecordView& recView) 
{
    auto i = recordMap.find(recView.getID());
    
    if (i != recordMap.end()) {
        return i->second;
    } else {
        return NO_VALUE;
    }
}

void RecordValues::fillValues() 
{
    fillTopModValue();
    fillValuesWithParent();
}

void RecordValues::print() 
{
    std::cout << "-------------- RecordValues ---------------" << std::endl;
    for (auto& i : recordMap) {
        const ObjectView& objView = elabDB->getObj(i.first);
        std::cout << objView.getDebugString() << " " << i.second.asString() << std::endl;
    }
}

/// Mapping record object ID to SValue for all records in the design
std::unordered_map<uint32_t, sc::SValue> RecordValues::recordMap;
ElabDatabase* RecordValues::elabDB;


} // end namespace sc_elab

