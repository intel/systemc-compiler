/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "ScObjectView.h"

#include <sc_tool/elab/ScElabDatabase.h>
#include <sc_tool/diag/ScToolDiagnostic.h>
#include <sc_tool/utils/DebugOptions.h>
#include <sc_tool/ScCommandLine.h>
#include <sc_tool/elab/ScVerilogModule.h>
#include <sc_tool/utils/ScTypeTraits.h>
#include <sc_tool/utils/CppTypeTraits.h>
#include <sc_tool/dyn_elab/GlobalContext.h>
#include <clang/AST/DeclTemplate.h>
#include <llvm/Support/Debug.h>
#include <variant>

using namespace sc;
using namespace llvm;
using std::cout; using std::endl;

namespace sc_elab
{

// Number of module and process instances, to print statistic
size_t modNum;
size_t procNum;
    
//  ObjectView -----------------------------------------------------------------

ObjectView::ObjectView(const sc_elab::Object *obj,
                       const sc_elab::ElabDatabase *db)
: obj(obj), db(db)
{
    SCT_TOOL_ASSERT (obj != nullptr, "Object is null");
    SCT_TOOL_ASSERT (db != nullptr, "DB is null");
}

bool operator == (const ObjectView& one, const ObjectView& other)
{
    return (one.obj == other.obj) && (one.db == other.db);
}
bool operator != (const ObjectView& one, const ObjectView& other)
{
    return !(one == other);
}

clang::QualType ObjectView::getType() const
{
    return db->getType(obj->type_id());
}

bool ObjectView::isPointerOrRef() const
{
    return isPrimitive() &&
        (primitive()->isPointer() || primitive()->isReference());
}

bool ObjectView::isChannel() const
{
    if (isSignal())
        return true;
    if (isPrimitive() && primitive()->isPort())
        return true;
    return false;
}

bool ObjectView::isTopMod() const
{
    return obj->id() == 0;
}

ElabObjVec ObjectView::getPointers() const
{
    ElabObjVec res;

    for (auto pointerID : obj->pointer_ids()) {
        ObjectView pointer = db->getObj(pointerID);
        if (!pointer.primitive()->isPort())
            res.push_back(pointer);
    }

    return res;
}

ElabObjVec ObjectView::getPorts() const
{
    ElabObjVec res;

    for (auto pointerID : obj->pointer_ids()) {
        ObjectView pointer = db->getObj(pointerID);
        if (pointer.primitive()->isPort())
            res.push_back(pointer);
    }

    return res;
}

ObjectView ObjectView::getParent() const
{
    SCT_TOOL_ASSERT (!isTopMod(), "");
    return db->getObj(obj->parent_ids(0));
}

ModuleMIFView ObjectView::getParentModule() const
{
    if(isTopMod())
        return *this;

    auto parent = getParent();
    while (!parent.isModule())
        parent = parent.getParent();

    return parent;
}

ModuleMIFView ObjectView::getParentModuleOrMIF() const
{
    if(isTopMod())
        return *this;

    auto parent = getParent();
    while (!(parent.isModule() || parent.isModularInterface()))
        parent = parent.getParent();

    return parent;
}


ModuleMIFView ObjectView::nearestCommonParentModule(ObjectView other) const
{
    auto thisParents = this->getParentModulesList();
    auto otherParents = other.getParentModulesList();

    const auto minSize = std::min(thisParents.size(), otherParents.size());

    for (size_t i = 0; i < minSize; ++i) {
        if (thisParents[i] != otherParents[i])
            return thisParents[i-1];
    }

    return thisParents[minSize-1];
}

std::deque<ModuleMIFView> ObjectView::getParentModulesList() const
{
    return getParentModulesList(*db->getObj(0).moduleMIF());
}

std::deque<ModuleMIFView>
ObjectView::getParentModulesList(const ModuleMIFView &rootMod) const
{
    std::deque<ModuleMIFView> res;
    auto parentMod = *this;
    do {
        parentMod = parentMod.getParentModule();
        res.push_front(parentMod);
    } while (parentMod != rootMod );

    return res;
}

std::optional<uint32_t> ObjectView::getElementIndex() const
{
    if (isArrayElement())
        return obj->array_idx();
    else
        return std::nullopt;
}

namespace {

struct PtrNode   {};
struct ArrNode   { uint32_t idx; };
struct FieldNode { const clang::FieldDecl *fieldDecl; };
struct BaseNode  { clang::QualType type; };
struct RootNode  { ObjectView root; };

using NodeT = std::variant<PtrNode,ArrNode,FieldNode,BaseNode, RootNode>;

std::deque<NodeT>  getHierarchyNodeVec(ObjectView obj)
{
    std::deque<NodeT>   resPath;

    /// Traverse the tree back to parent module
    while (!obj.isModule()) {
        if (obj.isDynamic()) {
            if (obj.getPointers().size() > 0) {
                obj = obj.getPointers()[0];
                resPath.emplace_front(PtrNode());
            } else if (obj.isArrayLike()) {
                // pointer to first element of array interpreted as array pointer
                auto firstEl = obj.array()->at(0);
                SCT_TOOL_ASSERT (firstEl.getPointers().size() == 1, "");
                obj = firstEl.getPointers()[0];
                resPath.emplace_front(PtrNode());
            } else {
                llvm_unreachable("ObjectView: Can not find pointer to object");
            }
        } else {

            if (obj.isArrayElement()) {
                resPath.emplace_front( ArrNode {*obj.getElementIndex()} );
                obj = obj.getParent();
            } else if (obj.isDataMember() || obj.isStatic()) {
                resPath.emplace_front( FieldNode{obj.getFieldDecl()} );
                obj =  obj.getParent();
            } else if (obj.isBaseClass()) {
                resPath.emplace_front( BaseNode{ obj.getType() } );
                obj =  obj.getParent();
            } else if (obj.hasNoParent()) {
                return {};
            } else {
                llvm_unreachable("ObjectView::selectAllSimilarArrayElements bad query");
            }
        }
    }

    resPath.emplace_front( RootNode{ obj } );
    return resPath;
}

void retriveObjectsRec(
    std::deque<NodeT> hierPath, ObjectView curObj, const IndexVec &indices,
    ArrayElemVec &resVec, bool getAllArrayElements)
{

    if (hierPath.empty()) {
        resVec.push_back({curObj, indices});
        
    } else {
        auto node = hierPath.front();
        hierPath.pop_front();

        if (std::get_if<PtrNode>(&node)) {
            auto ptr = *curObj.primitive()->ptrOrRef();
            if (auto pointee = ptr.pointeeOrArray() ) {
                retriveObjectsRec(hierPath, *pointee, indices, resVec, 
                                  getAllArrayElements);
            } else {
                return;
            }
        } else 
        if (std::get_if<ArrNode>(&node)) {
            auto arrayObj = *curObj.array();
            auto newIndicies = indices;
            newIndicies.push_back(0);

            for (unsigned i = 0; i < arrayObj.size(); ++i) {
                auto el = arrayObj.at(i);
                newIndicies.back() = i;
                retriveObjectsRec(hierPath, el, newIndicies, resVec, 
                                  getAllArrayElements);
                
                if (!getAllArrayElements) break;
            }
        } else 
        if (auto fieldNode = std::get_if<FieldNode>(&node)) {
            auto fieldObj = *curObj.record()->getField(fieldNode->fieldDecl);
            retriveObjectsRec(hierPath, fieldObj, indices, resVec,
                              getAllArrayElements);
            
        } else 
        if (auto baseNode = std::get_if<BaseNode>(&node)) {
            auto baseObj = *curObj.record()->getBase(baseNode->type);
            retriveObjectsRec(hierPath, baseObj, indices, resVec,
                              getAllArrayElements);
        }
    }
}

ArrayElemVec retriveObjects(std::deque<NodeT>  hierarchyVec,
                            bool getAllArrayElements)
{
    ArrayElemVec resVec;
    if (!hierarchyVec.empty()) {
        if (auto rootNode = std::get_if<RootNode>(&hierarchyVec.front())) {
            ModuleMIFView parentModule = rootNode->root;
            IndexVec indices;
            hierarchyVec.pop_front();
            retriveObjectsRec(hierarchyVec, parentModule,indices, resVec, 
                              getAllArrayElements);
        }
    }
    return resVec;
}

IndexVec retriveIndices(std::deque<NodeT>  hierarchyVec)
{
    IndexVec resVec;

    for (auto el : hierarchyVec) {
        if (auto arrNode = std::get_if<ArrNode>(&el)) {
            resVec.push_back(arrNode->idx);
        }
    }

    return resVec;
}

} // namespace

ArrayElemVec ObjectView::getAllSimilarArrayElements() const
{
    DEBUG_WITH_TYPE(DebugOptions::doElab, outs() << "SEARCH " << *this << "\n";);

    auto hierarchyVec = getHierarchyNodeVec(*this);

    DEBUG_WITH_TYPE(DebugOptions::doElab,
        outs() << "MOD";
        for (const auto &el : hierarchyVec ) {
            if (std::get_if<PtrNode>(&el)) {
                outs() << "<-";
            }
            else if (std::get_if<ArrNode>(&el)) {
                outs() << "[*]";
            }
            else if (auto fieldNode = std::get_if<FieldNode>(&el)) {
                outs() << "." << fieldNode->fieldDecl->getName();
            }
            else if (auto baseNode = std::get_if<BaseNode>(&el)) {
                outs() << "." << baseNode->type.getAsString();
            }
        }
        outs() << "\n";
    );

    auto resVec = retriveObjects(hierarchyVec, true);

    DEBUG_WITH_TYPE(DebugOptions::doElab,
        for (auto &el : resVec) {
            outs() << "FOUND: "<<  el.obj << "\n";
        }
    );

    return resVec;
}

ArrayElemObjWithIndices ObjectView::getAsArrayElementWithIndicies() const
{
    auto hierarchyVec = getHierarchyNodeVec(*this);
    IndexVec indices = retriveIndices(hierarchyVec);
    ObjectView element = retriveObjects(hierarchyVec, false)[0].obj;
    return ArrayElemObjWithIndices{element, indices};
}

ArrayElemObjWithIndices ObjectView::getAsArrayElementWithIndicies(PortView port) const
{
    if (isSignal() && isDynamic() && getPointers().size() == 0) {
        for (auto p : getPorts()) {
            if (p != port) {
                return p.getAsArrayElementWithIndicies();
            }
        }
        llvm_unreachable("No bound port found for dynamic signal");
    } else {
        return getAsArrayElementWithIndicies();
    }
}

PortView ObjectView::getPortBound(PortView port) const
{
    if (isSignal() && isDynamic() && getPointers().size() == 0) {
        for (auto p : getPorts()) {
            if (p != port) {
                return p;
            }
        }
        llvm_unreachable("No bound port found for dynamic signal");
    } else {
        return port;
    }
}

std::optional<ObjectView> ObjectView::derefRecursively() const
{
    std::optional<ObjectView> res = *this;

    while (res && res->isPointerOrRef()) {
        res = res->primitive()->ptrOrRef()->getFirstNonPointerPointee();
    }

    return res;
}

std::optional<ObjectView> ObjectView::getVerilogNameOwner() const
{
    if (auto arr = array()) {
        if (arr->hasElements()) {
            if (arr->isConstant())
                return *this;
            else
                return arr->getFirstNonArrayEl();
        } else
            return *this;
    }

    return this->derefRecursively();
}



clang::ValueDecl* ObjectView::getValueDecl() const
{
    if (isDataMember() || isStatic()) {
        auto cxxRecord = getParent().getType().getTypePtr()->getAsCXXRecordDecl();
        for (clang::Decl* decl : cxxRecord->decls()) {

            if (auto* valDecl = llvm::dyn_cast<clang::ValueDecl>(decl)) {

                if (valDecl->getNameAsString() == obj->field_name() )
                    return valDecl;
            }
        }

        llvm::outs() << "Can't find field: " << obj->field_name() << "\n";
        return nullptr;
    }

    return nullptr;
}

const std::string* ObjectView::getFieldName() const
{
    if (isDataMember() || isStatic() || (isPrimitive() && primitive()->isProcess())) {
        return &obj->field_name();
    }

    return nullptr;
}

const clang::FieldDecl* ObjectView::getFieldDecl() const
{
    if (isDataMember()) {
        const auto &fieldName = *getFieldName();
        auto *parentRecDecl = getParent().getType()->getAsCXXRecordDecl();
        for (const auto field: parentRecDecl->fields()) {
            if (field->getName() == fieldName) {
                return field;
            }
        }
    }

    return nullptr;
}


llvm::SmallVector<VerilogVarRef, 1> ObjectView::getVerilogVars() const
{
    auto mod = this->getParentModule();
    auto verilogMod = db->getVerilogModule(mod);
    
    llvm::SmallVector<VerilogVarRef, 1> res;

    if (this->isChannel()) {
        auto arrayEl = this->getAsArrayElementWithIndicies();
        auto vars = verilogMod->getVerVariables(arrayEl.obj);
        for (auto *var : vars) {
            res.emplace_back(var, arrayEl.indices);
        }
    } else {
        auto vars = verilogMod->getVerVariables(*this);
        for (auto *var : vars) {
            res.emplace_back(var, IndexVec{});
        }
    }

    return res;
}

// Do not change offset to zero as it done in getVerilogVars()
llvm::SmallVector<VerilogVarRef, 1> ObjectView::getVerilogVarsOffset() const
{
    auto mod = this->getParentModule();
    auto verilogMod = db->getVerilogModule(mod);
    
    llvm::SmallVector<VerilogVarRef, 1> res;

    auto vars = verilogMod->getVerVariables(*this);
    for (auto *var : vars) {
        res.emplace_back(var, IndexVec{});
    }

    return res;
}

std::optional<PrimitiveView> ObjectView::primitive() const
{
    if (isPrimitive())
        return PrimitiveView(*this);

    return std::nullopt;
}

std::optional<RecordView> ObjectView::record() const
{
    if (isRecord())
        return RecordView(*this);

    return std::nullopt;
}

std::optional<ModuleMIFView> ObjectView::moduleMIF() const
{
    if (isModule() || isModularInterface())
        return ModuleMIFView(*this);

    return std::nullopt;
}

std::optional<ArrayView> ObjectView::array() const
{
    if (isArrayLike())
        return ArrayView(*this);

    return std::nullopt;
}

std::optional<SignalView> ObjectView::signal() const
{
    if (isSignal())
        return SignalView(*this);

    return std::nullopt;
}

std::optional<std::string> ObjectView::string() const
{
    if (isPrimitive()) {
        if (obj->primitive().kind() == Primitive::STRING) {
            return obj->primitive().str_val();
        }
    }

    return std::nullopt;
}

std::string ObjectView::getDebugString() const
{
    std::string res;

    std::string kind;

    if (isArrayLike())
        kind = "[ARRAY  ]: ";
    else if(isModularInterface())
        kind = "[MOD_IF ]: ";
    else if(isModule())
        kind = "[MODULE ]: ";
    else if(isSignal())
        kind = "[SIGNAL ]: ";
    else if(isRecord())
        kind = "[RECORD ]: ";
    else if (isPrimitive()) {

        PrimitiveView prim = *primitive();

        if (prim.isPointer())
            kind = "[POINTER]: ";
        else if(prim.isReference())
            kind = "[REF    ]: ";
        else if(prim.isValue())
            kind = "[VALUE  ]: ";
        else if(prim.isPort())
            kind = "[PORT   ]: ";
        else if(prim.isProcess())
            kind = "[PROCESS]: ";
        else if(prim.isEvent())
            kind = "[EVENT  ]: ";
        else if(prim.isExport())
            kind = "[EXPORT]: ";
        else if(prim.isString())
            kind = "[STRING]: ";
        else if(prim.isUnsupported())
            kind = "[UNSUP ]: ";
        else
            SCT_TOOL_ASSERT (false, "Incorrect kind");
    } else
        SCT_TOOL_ASSERT (false, "Incorrect kind");

    auto parent = *this;

    while (!parent.isTopMod()) {
        if (parent.isDynamic())
            res = "[new " + parent.getType().getAsString() + "]" + res;
        else if (parent.isArrayElement())
            res = "["+std::to_string(*parent.getElementIndex())+"]"+ res;
        else if (parent.isDataMember())
            res = "."+*parent.getFieldName() + res;
        else if (parent.isStatic())
            res = ".(static)"+*parent.getFieldName() + res;
        else if (parent.isBaseClass())
            res = "::"+ parent.getType().getAsString() + res;
        else if (parent.hasNoParent()) {
            res = "[NO_PARENT]";
        } else {
            llvm_unreachable("Unknown rel type");
        }

        parent = parent.getParent();
    }

    res = kind + "  [TOP]" + res;

    return res;
}

static void dumpHierarchyRecursive(ObjectView obj, const std::string& prefix,
                                   bool traverseSubmods)
{
    if (obj.getID() == 0) {
        llvm::outs() << "[" << "TOP" << "] ";
    } else 
    if (obj.isArrayElement()) {
        llvm::outs() << "[" << *obj.getElementIndex() << "] ";
    } else 
    if (obj.isDataMember()) {
        llvm::outs() << "[" << obj.getType().getAsString() << " " 
                     << *obj.getFieldName() << "] ";
    } else 
    if (obj.isBaseClass()) {
        llvm::outs() << "[" << obj.getType().getAsString() << "] ";
    } else 
    if (obj.isDynamic()) {
        llvm::outs() << "[" << "DYN_ALLOC" << "] ";
    } else 
    if (obj.isStatic()) {
        llvm::outs() << "[" << "STATIC" << "] ";
    } else
        SCT_TOOL_ASSERT (false, "");

    if (obj.isProcess()) procNum++;
    
    llvm::outs() << "ID:" << obj.getID() << " ";

    auto getNewPrefixes = [&](bool isLast) {
        if (isLast)
            return std::make_pair("`---", prefix + "    ");
        else
            return std::make_pair("|---", prefix + "|   ");
    };

    if (obj.isRecord()) {

        if (obj.isModularInterface())
            llvm::outs() << "(MIF)\n";
        else if (obj.isModule()) {
            modNum++;
            llvm::outs() << "(MOD)\n";
            if (!traverseSubmods && !prefix.empty())
                return;
        }
        else if (obj.isSignal())
            llvm::outs() << "(SIG)\n";
        else
            llvm::outs() << "(REC)\n";

        ElabObjVec members;
        if (auto modMif = obj.moduleMIF())
            members = modMif->getAllMembers();
        else
            members = obj.record()->getMembers();

        for (size_t i = 0; i < members.size(); ++i) {
            auto newPrefixes = getNewPrefixes(i == members.size() - 1);

            llvm::outs() << prefix << newPrefixes.first;
            dumpHierarchyRecursive(members[i], newPrefixes.second,
                                   traverseSubmods);
        }

    } else 
    if (obj.isArrayLike()) {
        llvm::outs() << "(ARR)\n";

        for (size_t i = 0; i < obj.array()->size(); ++i) {
            auto newPrefixes = getNewPrefixes(i == obj.array()->size() - 1);

            auto element = obj.array()->at(i);
            llvm::outs() << prefix << newPrefixes.first;
            dumpHierarchyRecursive(element, newPrefixes.second,
                                  traverseSubmods);
        }

    } else {
        llvm::outs() << "(PRM) ";
        auto prim = *obj.primitive();

        if (auto ptr = prim.ptrOrRef()) {
            llvm::outs() << "PTR ";
            if (auto pointeeID = ptr->getPointeeID()) {
                llvm::outs() << "Pointee ID:" << *pointeeID;
                if (ptr->isBaseOffsetPtr())
                    llvm::outs() << ":" << *ptr->getOffset();
            }
            else {
                if (ptr->isNull())
                    llvm::outs() << "Null ";
                else
                    llvm::outs() << "Dangling ";
            }
        } else 
        if (prim.isProcess()) {
            if (auto proc = prim.process()) {
                llvm::outs() << proc->getDebugString();
            }
            
        } else 
        if (prim.isString()) {
            if (auto str = prim.string()) {
                llvm::outs() << "STR \"" << *str << "\"";
            }
            
        } else {
            // Print primitive value
            if (auto primValue = prim.value()) {
                if (auto intValue = primValue->int64Val()) {
                    llvm::outs() << (*intValue);
                    if (isCharType(obj.getType())) {
                        llvm::outs() << " " << char(*intValue);
                    }
                }
            }
            
        }

        llvm::outs() << "\n";
    }
}

void ObjectView::dumpHierarchy(bool traverseSubmods) const
{
    modNum = 0; procNum = 0;
    
    llvm::outs() << "HIERARCHY DUMP " << getDebugString() << "\n";
    dumpHierarchyRecursive(*this, "", traverseSubmods);

    llvm::outs() << "--------------------------------------\n" 
                 << "Module instance number " << modNum << "\n";
    llvm::outs() << "Process instance number " << procNum << "\n"
                 << "--------------------------------------\n";
}

std::optional<ObjectView> ObjectView::getTopmostParentArray() const 
{
    using std::cout; using std::endl;
    ObjectView parent;

    if (isDynamic()) {
        // Dynamically allocated signal with leaked pointer
        if (getPointers().size() == 0) {
            parent = getParent();
        } else {
            SCT_TOOL_ASSERT (getPointers().size() == 1, "Multiples pointers");
            parent = getPointers()[0];
        }
    } else {
        parent = getParent();
    }

    if (parent.isArrayLike()) {
        ObjectView parParent;
        
        if (parent.isDynamic()) {
            if(parent.getPointers().size() == 1)
                parParent = parent.getPointers()[0];
            else if (parent.array()->at(0).getPointers().size() == 1)
                parParent = parent.array()->at(0).getPointers()[0];
            else
                ScDiag::reportErrAndDie(
                    "Can't find unique owner for dynamic array" + 
                    parent.getDebugString() );

            if (parParent.isArrayElement()) {
                return parParent.getTopmostParentArray();
            } else {
                return parent;
            }
        } else {
            if (parent.getParent().isArrayLike()) {
                return parent.getTopmostParentArray();
            } else {
                return parent;
            }
        }
    } 
    
    if (parent.isPrimitive() && parent.primitive()->ptrOrRef()) {
        return parent.primitive()->ptrOrRef()->getTopmostParentArray();
    }
    
    return std::nullopt;
}

// Return topmost array or pointer variable 
std::optional<ObjectView> ObjectView::getTopmostParentArrayOrPointer() const 
{
    using std::cout; using std::endl;
    ObjectView parent;

    if (isDynamic()) {
        SCT_TOOL_ASSERT (getPointers().size() == 1, "Multiples pointers");
        parent = getPointers()[0];
    } else {
        parent = getParent();
    }
    //cout << "parent#1 " << parent.getDebugString() << endl;

    if (parent.isArrayLike()) {
        ObjectView parParent;
        
        if (parent.isDynamic()) {
            if(parent.getPointers().size() == 1)
                parParent = parent.getPointers()[0];
            else if (parent.array()->at(0).getPointers().size() == 1)
                parParent = parent.array()->at(0).getPointers()[0];
            else
                ScDiag::reportErrAndDie(
                    "Can't find unique owner for dynamic array" + 
                    parent.getDebugString() );

            if (parParent.isArrayElement()) {
                return parParent.getTopmostParentArray();
            } else {
                return parent;
            }
        } else {
            if (parent.getParent().isArrayLike()) {
                return parent.getTopmostParentArray();
            } else {
                return parent;
            }
        }
    } 
    
    if (parent.isPrimitive() && parent.primitive()->ptrOrRef()) {
        auto parentArray = parent.primitive()->ptrOrRef()->getTopmostParentArray();
        //cout << "prim #1 " << (parentArray ? parentArray->getDebugString() : "NO prim") << endl;
        
        if (parentArray) {
            // Array of pointers
            return *parentArray;
        } else {
            // Single pointer 
            return *parent.primitive();
        }
    }

    return std::nullopt;
}

// ArrayView -------------------------------------------------------------------

ArrayView::ArrayView(const ObjectView &parent)
    : ObjectView(parent)
{
    SCT_TOOL_ASSERT (isArrayLike(), "");
}

uint32_t ArrayView::size() const
{
    return getProtobufObj()->array().element_ids().size();
}

bool ArrayView::hasElements() const
{
    return !getProtobufObj()->array().element_ids().empty();
}

std::vector<size_t> ArrayView::getOptimizedArrayDims() const
{
    std::vector<size_t> res;

    SCT_TOOL_ASSERT (!hasElements(), "");
    for (auto dim : getProtobufObj()->array().dims())
        res.push_back(dim);

    return res;
}

std::size_t ArrayView::getOptimizedArrayBitwidth() const
{
    SCT_TOOL_ASSERT (!hasElements(), "");
    return getProtobufObj()->primitive().init_val().bitwidth();
}

std::optional<ObjectView> ArrayView::getFirstNonArrayEl() const
{
    ObjectView el = at(0);

    if (std::optional<PrimitiveView> prim = el.primitive()) {
        if (std::optional<PtrOrRefView> ptr = prim->ptrOrRef()) {
            if (auto pointee = ptr->getFirstNonPointerPointee()) {
                el = *pointee;
            } else
                return std::nullopt;
        }
    }

    if (auto arrayEl = el.array()) {
        if (arrayEl->hasElements())
            return arrayEl->getFirstNonArrayEl();
        else
            return *arrayEl;
    } else {
        return el;
    }
}

bool ArrayView::isChannelArray() const
{
    if (!hasElements())
        return false;

    ObjectView el = at(0);

    if (std::optional<PrimitiveView> prim = el.primitive()) {
        if (std::optional<PtrOrRefView> ptr = prim->ptrOrRef()) {
            if (auto pointee = ptr->getFirstNonPointerPointee()) {
                el = *pointee;
            } else
                return false;
        }
    }

    if (auto arrayEl = el.array()) {
        return arrayEl->isChannelArray();
    } else {
        return el.isChannel();
    }
}

bool ArrayView::isConstPrimitiveArray() const
{
    if (!isConstant())
        return false;

    if (!hasElements())
        return false;

    auto elemObj = at(0);

    if (auto primitiveView = elemObj.primitive())
        return primitiveView->isValue();

    if (auto arrayView = elemObj.array())
        return arrayView->isConstPrimitiveArray();

    return false;
}


ObjectView ArrayView::at(std::size_t idx) const
{
    SCT_TOOL_ASSERT (idx < (std::size_t)getProtobufObj()->array().
                     element_ids_size(), "");
    uint32_t elemID = getProtobufObj()->array().element_ids(idx);
    return getDatabase()->getObj(elemID);
}

// RecordView ------------------------------------------------------------------

RecordView::RecordView(const ObjectView &parent)
    : ObjectView(parent)
{
    SCT_TOOL_ASSERT (isRecord(), "");
}

std::optional<ObjectView> RecordView::getField(const clang::FieldDecl *fieldDecl) const
{
    for (auto fieldView : getFields()) {
        if (fieldDecl->getName().equals(*fieldView.getFieldName()))
            return fieldView;
    }

    return std::nullopt;
}

std::optional<ObjectView> RecordView::getField(const std::string fieldName) const
{
    for (auto fieldView : getFields()) {
        if (*fieldView.getFieldName() == fieldName)
            return fieldView;
    }

    return std::nullopt;
}

std::optional<ObjectView> RecordView::getBase(clang::QualType baseType) const
{
    for (auto baseView : getBases()) {
        if (baseView.getType() == baseType)
            return baseView;
    }

    return std::nullopt;
}


std::vector<RecordView> RecordView::getBases() const
{
    std::vector<RecordView> basesVec;

    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);

        if (memberObj.isBaseClass())
            basesVec.push_back(*memberObj.record());
    }

    return basesVec;
}

ElabObjVec RecordView::getFields() const
{
    ElabObjVec fieldsVec;

    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);

        if (memberObj.isDataMember() || memberObj.isStatic())
            fieldsVec.push_back(memberObj);
    }

    return fieldsVec;
}

void RecordView::getFieldsNoZero(ElabObjVec& fieldsVec) const
{
    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);
        //cout << "  " << memberObj.getDebugString() << endl;

        if (memberObj.isDataMember() || memberObj.isStatic()) {
            const clang::QualType& type = memberObj.getType();
            if (!sc::isZeroWidthType(type) && !sc::isZeroWidthArrayType(type)) {
                fieldsVec.push_back(memberObj);
            }
        } else 
        if (memberObj.isRecord()) {
            memberObj.record()->getFieldsNoZero(fieldsVec);
        }   
    }
}

ElabObjVec RecordView::getStaticFields() const {
    ElabObjVec staticVec;

    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);

        if (memberObj.isStatic())
            staticVec.push_back(memberObj);
    }

    return staticVec;
}


ElabObjVec RecordView::getMembers() const
{
    ElabObjVec membersVec;

    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);

        if (memberObj.isDataMember() || memberObj.isBaseClass()
            || memberObj.isStatic())
            membersVec.push_back(memberObj);
    }

    return membersVec;
}

// ModuleMIFView  --------------------------------------------------------------

ModuleMIFView ::ModuleMIFView (const ObjectView &parent)
    : RecordView(parent)
{
    SCT_TOOL_ASSERT (isModule() || isModularInterface(), "");
}

std::vector<ProcessView> ModuleMIFView::getProcesses() const
{
    std::vector<ProcessView> processVec;

    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);

        if (auto prim = memberObj.primitive()) {
            if (auto proc = prim->process())
                processVec.push_back(*proc);
        }
    }
    return processVec;
}

ElabObjVec ModuleMIFView::getDynamics() const
{
    ElabObjVec dynamicsVec;

    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);
        if (memberObj.isDynamic())
            dynamicsVec.push_back(memberObj);
    }

    return dynamicsVec;
}

ElabObjVec ModuleMIFView::getAllMembers() const
{
    ElabObjVec membersVec;

    for (auto memberID : getProtobufObj()->record().member_ids()) {
        auto memberObj = getDatabase()->getObj(memberID);
        membersVec.push_back(memberObj);
    }

    return membersVec;
}

const VerilogModule * ModuleMIFView::getVerilogModule() const
{
    return getDatabase()->getVerilogModule(*this);
}

// SignalView  -----------------------------------------------------------------

SignalView::SignalView(const ObjectView &parent)
    : ObjectView(parent)
{
    SCT_TOOL_ASSERT (isSignal(), "");
}

ObjectView SignalView::getSignalValue()
{
    return getDatabase()->getObj(getProtobufObj()->record().member_ids(0));
}

//  PrimitiveView --------------------------------------------------------------

PrimitiveView::PrimitiveView(const ObjectView &parent)
    : ObjectView(parent)
{
    SCT_TOOL_ASSERT (isPrimitive(), "");
}

std::optional<ValueView> PrimitiveView::value() const
{
    if (isValue())
        return ValueView(*this);

    return std::nullopt;
}

std::optional<PtrOrRefView> PrimitiveView::ptrOrRef() const
{
    if (isPointer() || isReference())
        return PtrOrRefView(*this);

    return std::nullopt;
}

std::optional<PortView> PrimitiveView::port() const
{
    if (isPort())
        return PortView(*this);

    return std::nullopt;
}

std::optional<ProcessView> PrimitiveView::process() const
{
    if (isProcess())
        return ProcessView(*this);

    return std::nullopt;
}

//  ValueView -----------------------------------------------------------------
ValueView::ValueView(const ObjectView &parent)
    : PrimitiveView(parent)
{
    SCT_TOOL_ASSERT (isValue(), "");
}

std::size_t ValueView::bitwidth() const
{
    return getProtobufObj()->primitive().init_val().bitwidth();
}

bool ValueView::isDynamicBitwidth() const
{
    return getProtobufObj()->primitive().init_val().dyn_bitwidth();
}

std::optional<int64_t> ValueView::int64Val() const
{
    if (getProtobufObj()->primitive().init_val().has_int64_value())
        return getProtobufObj()->primitive().init_val().int64_value();

    return std::nullopt;
}

std::optional<uint64_t> ValueView::uint64Val() const
{
    if (getProtobufObj()->primitive().init_val().has_uint64_value())
        return getProtobufObj()->primitive().init_val().uint64_value();

    return std::nullopt;
}

std::optional<double> ValueView::doubleVal() const
{
    if (getProtobufObj()->primitive().init_val().has_double_val())
        return getProtobufObj()->primitive().init_val().double_val();

    return std::nullopt;
}

//  PtrOrRefView --------------------------------------------------------------

PtrOrRefView::PtrOrRefView(const ObjectView &parent)
    : PrimitiveView(parent)
{
    SCT_TOOL_ASSERT (isPointer() || isReference(), "");
    SCT_TOOL_ASSERT (getProtobufObj()->primitive().ptr_val().
                     pointee_id_size() <= 2, "");
}

bool PtrOrRefView::isNotNullDangling() const
{
    if (isNull())
        return false;

    return getProtobufObj()->primitive().ptr_val().pointee_id_size() == 0;
}

bool PtrOrRefView::isBaseOffsetPtr() const
{
    return getProtobufObj()->primitive().ptr_val().pointee_id_size() == 2;
}

std::optional<ObjectView> PtrOrRefView::pointee() const
{
    if (getProtobufObj()->primitive().ptr_val().pointee_id().size() == 1)
        return getDatabase()->getObj(getProtobufObj()->primitive().ptr_val().pointee_id(0));

    return std::nullopt;
}

std::optional<ObjectView> PtrOrRefView::pointeeOrArray() const
{
    auto &pointee_ids = getProtobufObj()->primitive().ptr_val().pointee_id();

    if (isBaseOffsetPtr()) {
//        for (size_t i = 1; i < pointee_ids.size(); ++i) {
//            if (pointee_ids[i] != 0)
//                return std::nullopt;
//        }
        return getDatabase()->getObj(pointee_ids[0]);
        
    } else {
        auto resPointee = pointee();
        if (resPointee && resPointee->isArrayElement()) {
            if (*resPointee->getElementIndex() == 0) {
                resPointee = resPointee->getParent();
            } else {
                return resPointee;
            }
        }

        return resPointee;
    }
}

std::optional<uint32_t> PtrOrRefView::getPointeeID() const
{
    if (getProtobufObj()->primitive().ptr_val().pointee_id_size() > 0)
        return getProtobufObj()->primitive().ptr_val().pointee_id(0);
    return std::nullopt;
}

std::optional<uint32_t> PtrOrRefView::getOffset() const
{
    SCT_TOOL_ASSERT (isBaseOffsetPtr(), "");

    if (getProtobufObj()->primitive().ptr_val().pointee_id().size() == 2)
        return getProtobufObj()->primitive().ptr_val().pointee_id(1);

    return std::nullopt;
}

std::optional<ObjectView> PtrOrRefView::getFirstNonPointerPointee() const {

    if (isBaseOffsetPtr())
        return pointeeOrArray();

    if(std::optional<ObjectView> pointeeObj = pointee()) {
        if (auto prim = pointeeObj->primitive()) {
            if (prim->isPointer())
                return prim->ptrOrRef()->getFirstNonPointerPointee();
        }
        return pointeeObj;
    }

    return std::nullopt;
}

//  PortView --------------------------------------------------------------

PortView::PortView(const ObjectView &parent)
    : PrimitiveView(parent)
{
    SCT_TOOL_ASSERT (isPort(), "");
}

bool PortView::isSignalPort() const
{
    return (getDirection() != PortDirection::NONE);
}

SignalView PortView::getBindedSignal() const
{
    auto bind = getDirectBind();
    while(!bind.isSignal()) {
        bind = bind.primitive()->port()->getDirectBind();
    }

    return bind;
}

ObjectView PortView::getDirectBind() const
{
    SCT_TOOL_ASSERT (getProtobufObj()->primitive().ptr_val().pointee_id_size() == 1,
                     "No single pointee object found for port binding");
    return getDatabase()->getObj(getProtobufObj()->primitive().ptr_val().
                                 pointee_id(0));
}

PortDirection PortView::getDirection() const
{
    if (!dirInitialized) {
        auto typeName = getType().getAsString();
        llvm::StringRef ty(typeName);

        if (ty.contains("sc_core::sc_out<") || ty.contains("sct::sct_out<"))
            dir =  PortDirection::OUT;
        else 
        if (ty.contains("sc_core::sc_in<") || ty.contains("sct::sct_in<"))
            dir = PortDirection::IN;
        else 
        if (ty.contains("sc_core::sc_inout<")) {
            ScDiag::reportErrAndDie("inout ports are not supported");
            dir = PortDirection::INOUT;
        }
        else
            dir = PortDirection::NONE;
    }

    return dir;
}

std::optional<ObjectView> PortView::pointee() const
{
    if (getProtobufObj()->primitive().ptr_val().pointee_id().size() == 1)
        return getDatabase()->getObj(getProtobufObj()->primitive().ptr_val().pointee_id(0));

    return std::nullopt;
}



//  ProcessView --------------------------------------------------------------

ProcessView::ProcessView(const ObjectView &parent)
    : PrimitiveView(parent)
{
    SCT_TOOL_ASSERT (isProcess(), "");
}

bool sc_elab::ProcessView::isScMethod() const
{
    return getProtobufObj()->primitive().proc_val().kind() == Process::SC_METHOD;
}

bool ProcessView::isScThread() const
{
    return getProtobufObj()->primitive().proc_val().kind() == Process::SC_THREAD;
}

bool ProcessView::isScCThread() const
{
    return getProtobufObj()->primitive().proc_val().kind() == Process::SC_CTHREAD;
}

// Does not work well for methods in MIF
bool ProcessView::isCombinational() const
{
    bool isCombinational = isScMethod();

    for (auto sens : staticSensitivity()) {
        if (!sens.isDefault())
            isCombinational = false;
    }

    return isCombinational;
}

std::vector<ProcessView::SensEvent> ProcessView::staticSensitivity() const
{
    std::vector<ProcessView::SensEvent> res;

    for (auto &event: getProtobufObj()->primitive().proc_val().static_events()) {
        res.push_back({getDatabase()->getObj(event.event_source_id()), event.kind()});
    }

    return res;
}

std::vector<ProcessView::Reset> ProcessView::resets() const
{
    std::vector<ProcessView::Reset> res;

    for (auto &reset: getProtobufObj()->primitive().proc_val().resets()) {
        res.emplace_back(getDatabase()->getObj(reset.source_id()),
                         reset.level(),
                         reset.async());
    }

    return res;
}


static std::optional<RecordView> findBaseClassByType(RecordView recView,
                                      clang::QualType baseType) {

    if (recView.getType() == baseType)
        return recView;

    for (auto baseClassView : recView.getBases())
        if (auto res = findBaseClassByType(baseClassView, baseType))
            return res;

    return std::nullopt;
}

// Get host class and function declarations for given process name and 
// class where process run by SC_METHOD/SC_CTHREAD
std::pair<clang::CXXRecordDecl*, clang::CXXMethodDecl*>
ProcessView::getProcDeclFromBase(clang::CXXRecordDecl* moduleDecl,
                                 const std::string& procName) const 
{
    for (clang::CXXMethodDecl* procDecl : moduleDecl->methods()) {
        if (procDecl->getDeclName().getAsString() == procName) {
            return {moduleDecl, procDecl};
        }
    }

    for (auto baseSpecf : moduleDecl->bases()) {
        if (auto baseDecl = baseSpecf.getType()->getAsCXXRecordDecl()) {
            auto entry = getProcDeclFromBase(baseDecl, procName);
            if (entry.first) {
                return entry;
            }
        }
    }
    return {nullptr, nullptr};
}

std::pair<RecordView, clang::CXXMethodDecl*> ProcessView::getLocation() const
{
    using namespace clang;
    ModuleMIFView parentModule = getParent();

    auto methodName = getProtobufObj()->field_name();
    auto typeName = getProtobufObj()->primitive().proc_val().type_name();
    QualType hostType = getMangledTypeDB()->getType(typeName);
    
//    std::cout << "\nthis process " << getDebugString() 
//              << "\nparentModule " << parentModule.getDebugString()
//              << "\nhostType " << hostType.getAsString() 
//              << "\nmethodName " << methodName << "\n";

    if (auto hostClass = findBaseClassByType(parentModule, hostType)) {
        // Get process host class declaration and process declaration     
        // Process can be declared of base class and run from inheritor
        CXXRecordDecl* hostRecDecl = hostClass->getType()->getAsCXXRecordDecl();
        auto entry = getProcDeclFromBase(hostRecDecl, methodName);
        
        if (entry.first) {
            // Get RecordView for process host class, may be it is base class 
            QualType procClassType = QualType(entry.first->getTypeForDecl(), 0);

            if (auto procClass = findBaseClassByType(parentModule, procClassType)) {
                clang::CXXMethodDecl* procDecl = entry.second;
                return {*procClass, procDecl};
            }
        }
        SCT_TOOL_ASSERT (false, "Process declaration not found");
        
    } else {
        ScDiag::reportScDiag(ScDiag::SC_NO_MODULE_NAME);
        SCT_TOOL_ASSERT (false, "No hostClass for process");
    }
    
    llvm_unreachable("Error in ProcessView::getLocation()");
}

} // end namespace sc_elab


