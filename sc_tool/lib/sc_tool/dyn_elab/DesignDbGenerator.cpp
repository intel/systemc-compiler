/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/dyn_elab/DesignDbGenerator.h>
#include <sc_tool/dyn_elab/Demangle.h>
#include <sc_tool/dyn_elab/Reflection.h>
#include <sc_tool/dyn_elab/GlobalContext.h>
#include <sc_tool/diag/ScToolDiagnostic.h>
#include <sc_tool/utils/DebugOptions.h>
#include <sc_tool/utils/CppTypeTraits.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecordLayout.h>
#include <clang/AST/VTableBuilder.h>
#include <google/protobuf/text_format.h>
#include <rtti_sysc/SystemCRTTI.h>

#include <typeinfo>
#include <sc_elab/allocated_node.h>
#include "DesignDbGenerator.h"

using namespace clang;
using namespace sc;

namespace sc_elab
{

static bool isPrimitive(const Object &obj) {
    return obj.kind() == Object::PRIMITIVE;
}

static bool isRecord(const Object &obj) {
    return obj.kind() == Object::RECORD;
}

static bool isArray(const Object &obj) {
    return obj.kind() == Object::ARRAY;
}

static bool isAggregate(const Object &eo) {
    return isRecord(eo) || isArray(eo);
}

static bool isPort(const Object &obj) {
    return isPrimitive(obj) && obj.primitive().kind() == Primitive::PORT;
}

static bool isValue(const Object &obj) {
    return isPrimitive(obj) && obj.primitive().kind() == Primitive::VALUE;
}

static bool isProcess(const Object &obj) {
    return isPrimitive(obj) && obj.primitive().kind() == Primitive::PROCESS;
}

static bool isSignal(const Object &obj) {
    return obj.sckind() == Object::SC_SIGNAL;
}

static bool isModule(const Object &obj) {
    return obj.sckind() == Object::SC_MODULE
    || obj.sckind() == Object::SC_MODULAR_INTERFACE;
}


void ObjectMap::addObject(TypedObject to, Object *o)
{
    rfl2ElabMap.emplace(to.getUnqualified(), o);
    elab2RflMap.emplace(o, to);
    allTOs.push_back(to.getUnqualified());
}

Object *ObjectMap::findElabObj(TypedObject to)
{
    auto iter = rfl2ElabMap.find(to.getUnqualified());

    if (iter != rfl2ElabMap.end()) {
        return iter->second;
    }

    return nullptr;
}

std::optional<TypedObject> ObjectMap::findTypedObj(const Object *eo) const
{
    auto iter = elab2RflMap.find(eo);

    if (iter != elab2RflMap.end()) {
        return iter->second;
    }
    return std::nullopt;
}


static bool typesMatch(TypedObject ptr, TypedObject target) {
    auto unQualTargetType = target.getType().getUnqualifiedType();

    if (ptr.isRecordObject() && target.isRecordObject() &&
        ptr.getAs<RecordObject>()->isPolymorphic()) {
        // For polymorphic objects compares dynamic type
        auto ptrRec = ptr.getType()->getAsCXXRecordDecl();
        auto targetRec = unQualTargetType->getAsCXXRecordDecl();

        if (targetRec->isDerivedFrom(ptrRec) || targetRec == ptrRec) {
            auto dynType = ptr.getAs<RecordObject>()
                            ->getDynamicType().getUnqualifiedType();

            return dynType == unQualTargetType;
        }

    } else {
        auto unqualPtr = ptr.getType().getUnqualifiedType();

        return unqualPtr == unQualTargetType;
    }

    return false;
}

std::optional<std::pair<Object *, size_t>>
ObjectMap::resolvePointer(PtrOrRefObject ptrObj)
{

    if (ptrObj.isNullPtr()) {
        return std::nullopt;
    }
    
    // Dereference a pointer to get a possibly invalid 
    // (if pointer is dangling) typed object
    TypedObject possiblePointee = ptrObj.dereference().getUnqualified();

    // Find all objects that overlap with given address in memory
    std::vector<TypedObject> typedObjsAtAddr;
    uintptr_t thisPtr = (uintptr_t)possiblePointee.getPtr();
    for (TypedObject & to : allTOs) {
        uintptr_t startPtr = (uintptr_t)to.getPtr();
        uintptr_t endPtr = startPtr + to.getSizeInBytes();

        if (thisPtr >= startPtr && thisPtr < endPtr) {
            typedObjsAtAddr.push_back(to);
        }
    }

    if (typedObjsAtAddr.empty()) {
        return std::nullopt;
    
    } else {

        for (auto to : typedObjsAtAddr) {
            // If type of pointer matches with type of object, return object
            if (typesMatch(possiblePointee, to))
                return std::pair<Object *, size_t>{rfl2ElabMap.at(to), 0};

            // A pointer can point to element of array, for example
            // int *x = &array[1][2];
            // since we don't store elements of non-constant integer arrays
            // we need to resolve such a pointer as a (array + offset)
            if (auto arrayPointee = to.getAs<ArrayObject>()) {
                auto eo = rfl2ElabMap.at(to);

                // if array has elements in db,
                // than pointer will be resolved to array element
                if (eo->array().element_ids_size() != 0)
                    continue;

                // get array element that matches pointer type
                while (arrayPointee) {
                    if (typesMatch(possiblePointee, (*arrayPointee)[0])) {

                        size_t arrayPtr = (size_t)arrayPointee->getPtr();
                        size_t sizeOfEl = possiblePointee.getSizeInBytes();

                        uintptr_t offset = (thisPtr - arrayPtr) / sizeOfEl;

                        assert(((thisPtr - arrayPtr) % sizeOfEl) == 0);

                        return std::pair<Object *, size_t>{
                                    rfl2ElabMap.at(to), offset};
                    }

                    arrayPointee = (*arrayPointee)[0].getAs<ArrayObject>();
                }

            }
        }
    }

    return std::nullopt;
}


void sc_elab::DesignDbGenerator::run(clang::CXXRecordDecl* modRecordDecl,
                                   void* objectPtr)
{
    TypedObject tObj{objectPtr, modRecordDecl->getTypeForDecl()->
                     getCanonicalTypeInternal()};

    auto scModObj = *tObj.getAs<ScObject>();

    // Create Top-level module object, always have ID == 0
    auto topObj = createElabObject(tObj, 0, false);
    topObj->set_rel_type(Object::DYNAMIC); // means nothing in this case
    topObj->set_kind(Object::RECORD);
    topObj->set_sckind(Object::SC_MODULE);
    topObj->set_sc_name(scModObj.scName());

    designDB.add_module_ids(topObj->id());

    // Create children EOs recursively
    addChildrenRecursive(tObj, topObj);

    // Create pointer-pointee links, bind ports to ports and signals
    resolvePointers();
    
    // Checking not connected ports
    bool notBoundPortFound = false;
    
    for (int i = 0; i < designDB.objects_size(); ++i) {
        Object* obj = designDB.mutable_objects(i);
        const Primitive& prim = obj->primitive();
        
        if (prim.kind() == Primitive::PORT) {
            if (!prim.has_ptr_val() || prim.ptr_val().is_null()) {
                notBoundPortFound = true;
                bool fieldNameFound = true;

                // Find field name for array of/record with ports
                while (!obj->has_field_name()) {
                    if (obj->rel_type() == Object::DYNAMIC) {
                        // Get pointer to dynamic object
                        if (obj->pointer_ids_size() > 0) {
                            obj = designDB.mutable_objects(obj->pointer_ids(0));
                        } else {
                            fieldNameFound = false; break;
                        }
                    } else {
                        if (obj->parent_ids_size() > 0) {
                            obj = designDB.mutable_objects(obj->parent_ids(0));
                        } else {
                            fieldNameFound = false; break;
                        }
                    }
                    
                    if (obj->sckind() == Object::SC_MODULE ||
                        obj->sckind() == Object::SC_MODULAR_INTERFACE) {
                        fieldNameFound = false; break;
                    }
                }

                std::string fieldName = (fieldNameFound) ? 
                                        obj->field_name() : "No field name";
                std::string portName = (obj->has_sc_name()) ? 
                                        obj->sc_name() : "No SC name";
                ScDiag::reportScDiag(ScDiag::SC_PORT_NOT_BOUND) 
                                      << fieldName << portName;
            }
        }
    }
    
    if (notBoundPortFound) {
        ScDiag::reportScDiag(ScDiag::SC_PORT_NOT_BOUND);
        SCT_INTERNAL_FATAL_NOLOC ("Port not bound");
    }
    
    // Add process sensitivity lists
    resolveSensitivity();
    // Add process resets
    resolveResetFinders();

    // Some debug output
    std::string designDBStr;
    if (google::protobuf::TextFormat::PrintToString(designDB, &designDBStr)) {
        if (DebugOptions::isEnabled(DebugComponent::doElab)) {
            std::cout << "Design DB:\n" << designDBStr;

            std::cout << "designDB.objects_size " << designDB.objects_size() << "\n";
            std::cout << "designDB.module_ids_size " << designDB.module_ids_size() << "\n";
            
            int n_procs = 0;
            for (const auto & obj : designDB.objects()) {
                if (obj.kind() == Object::PRIMITIVE && obj.primitive().kind() == Primitive::PROCESS)
                    n_procs ++;
            }

            std::cout << "designDB.n_procs " << n_procs << "\n";
        }
    } else {
        std::cerr << "Invalid Design DB, partial content: "
                  << designDB.ShortDebugString() << "\n";
    }

    // Serialization to check validity message correctness, no store to file
    designDB.SerializeAsString();
}


template <class FieldT>
Object* DesignDbGenerator::createFieldElabObject(
    FieldT fieldObj, Object* parentElabObj, bool isBaseClass) {

    bool isConst = parentElabObj->is_constant() ||
        fieldObj.typedObj.getType().isConstQualified();

    auto fieldElabObj = createElabObject(fieldObj.typedObj,
                                         parentElabObj->id(), isConst);

    if (isBaseClass) {
        fieldElabObj->set_rel_type(Object::BASE_CLASS);
    } else {
        fieldElabObj->set_rel_type(Object::DATA_MEMBER);
    }

    parentElabObj->mutable_record()->add_member_ids(fieldElabObj->id());
    fillElabObject(fieldElabObj, fieldObj.typedObj);
    return fieldElabObj;
}

void DesignDbGenerator::addChildrenRecursive(TypedObject hostTO, Object* hostEO)
{

    // If object is array, create array elements
    if (auto arrayObj = hostTO.getAs<ArrayObject>()) {

        bool isConstArray = hostTO.getType().isConstQualified() ||
                            hostEO->is_constant();
        bool intArray = arrayObj->size() != 0 && 
                        isAnyInteger(arrayObj->getInnerElementType());

        // For efficiency, array elements for non-const integer arrays are not created
        // TODO: support small array here
        bool addArrayElements = isConstArray || !intArray;

        if (addArrayElements) {

            // Create EO for each array element
            for (size_t i = 0; i < arrayObj->size(); ++i) {

                auto elTO = (*arrayObj)[i];
                auto elEO = createArrayElementObject(elTO, i, hostEO);

                if (isAggregate(*elEO)) {
                    addChildrenRecursive(elTO, elEO);
                }
            }

        } else {
            // Create dummy primitive inside array to hold bitwidth
            if (arrayObj->size() != 0) {
                auto intObj = arrayObj->getFirstInnerElement().getAs<IntegerObject>();
                hostEO->mutable_primitive()->set_kind(Primitive::VALUE);
                fillIntValue(hostEO, *intObj);
            }
        }
    }

    // If object is signal, create value object
    else if (isScSignal(hostTO.getType())) {

        // m_cur_val data member holds current signal value
        auto curValField = hostTO.getAs<RecordObject>()->findField("m_cur_val");
        auto valEO = createElabObject(curValField->typedObj, hostEO->id(), 
                                      hostEO->is_constant());
        valEO->set_rel_type(Object::DATA_MEMBER);
        valEO->set_field_name("m_cur_val");
        hostEO->mutable_record()->add_member_ids(valEO->id());

        fillElabObject(valEO, curValField->typedObj);

        // If T of signal <T> is not integer primitive, recursively create it's data members
        if (isAggregate(*valEO))
            addChildrenRecursive(curValField->typedObj, valEO);
    }

    // If object is record, create bases and data members
    else if (auto recordObject = hostTO.getAs<RecordObject>()) {

        // Create all base class EOs
        for (auto base : recordObject->bases()) {

            // Ignore SystemC internals, like sc_module, sc_object...
            if (!isScCoreType(base.typedObj.getType())) {

                // If base is virtual, it may be already created
                if (base.itemDecl.isVirtual()) {
                    if (auto virtBaseObj = memMap.findElabObj(base.typedObj)) {
                        virtBaseObj->add_parent_ids(hostEO->id());
                        hostEO->mutable_record()->add_member_ids(virtBaseObj->id());
                        continue;
                    }
                }

                // Create base EO
                auto elabObj = createFieldElabObject(base, hostEO, true);

                elabObj->set_field_name(base.itemDecl.getType().getAsString());

                // Recursively create base EO data members
                if (isAggregate(*elabObj))
                    addChildrenRecursive(base.typedObj,elabObj);
            }
        }

        // Create all fields EOs
        for (auto field: recordObject->fields()) {

            auto elabObj = createFieldElabObject(field, hostEO, false);

            elabObj->set_field_name(field.itemDecl->getName().str());

            if (isAggregate(*elabObj))
                addChildrenRecursive(field.typedObj, elabObj);
        }

    } else {
        // not an aggregate
        return;
    }

    // If module, create EOs for dynamic allocations
    if (isModule(*hostEO)) {
        addModuleDynamicAllocs(hostTO, hostEO);
    }

}

Object *DesignDbGenerator::createElabObject(TypedObject to,
                                         DesignDbGenerator::ID parentID,
                                         bool isConst)
{
    ID typeID = typeManager.getOrCreateTypeID(to.getType());
    Object* newObject = designDB.add_objects();

    memMap.addObject(to, newObject);

    newObject->set_id(designDB.objects_size() - 1);
    newObject->set_type_id(typeID);
    newObject->add_parent_ids(parentID);
    newObject->set_is_constant(isConst);
    newObject->set_sckind(Object::SC_NONE);

    return newObject;
}

void DesignDbGenerator::fillElabObject(Object *elabObj,
                                     TypedObject typedObj)
{

    if (auto arrayObject = typedObj.getAs<ArrayObject>()) {

        elabObj->set_kind(Object::ARRAY);
        for (auto dim: arrayObject->dimensions()) {
            elabObj->mutable_array()->add_dims(static_cast<uint32_t>(dim));
        }
        if (arrayObject->getKind() == ArrayObject::SC_VECTOR) {
            elabObj->set_sckind(Object::SC_VECTOR);
        }

    } else 
    if (auto ptrOrRefObj = typedObj.getAs<PtrOrRefObject>()) {

        const QualType &qualType = typedObj.getType();
        elabObj->set_kind(Object::PRIMITIVE);

        if (ptrOrRefObj->isPointer()) {
            if (isConstCharPtr(qualType)) {
                // Consider const char* as string
                elabObj->mutable_primitive()->set_kind(Primitive::STRING);
                std::string* str_val = elabObj->mutable_primitive()->mutable_str_val();
                auto A = reinterpret_cast<const char* const*>(typedObj.getPtr());
                // Check const char * is @nullptr
                if (*A) {
                    *str_val = std::string(*A);
                } else {
                    *str_val = "";
                }
            } else {
                elabObj->mutable_primitive()->set_kind(Primitive::POINTER);
            }
        } else {
            elabObj->mutable_primitive()->set_kind(Primitive::REFERENCE);
        }

    } else 
    if (auto intObj = typedObj.getAs<IntegerObject>()) {

        elabObj->set_kind(Object::PRIMITIVE);
        elabObj->mutable_primitive()->set_kind(Primitive::VALUE);
        fillIntValue(elabObj, *intObj);

    } else 
    if (auto recObj = typedObj.getAs<RecordObject>()) {

        const QualType &qualType = typedObj.getType();
        elabObj->set_kind(Object::RECORD);

        if (auto scObject = typedObj.getAs<ScObject>()) {

            elabObj->set_sc_name(scObject->scName());

            if (isScBasePort(qualType)) {
                elabObj->set_kind(Object::PRIMITIVE);
                elabObj->mutable_primitive()->set_kind(Primitive::PORT);
            }
            else if (isScProcess(qualType)) {
                elabObj->set_kind(Object::PRIMITIVE);
                elabObj->mutable_primitive()->set_kind(Primitive::PROCESS);
                auto *proc_val = elabObj->mutable_primitive()->mutable_proc_val();
                auto procName =
                    getProcessTypeInfo((const sc_core::sc_object *) typedObj.getPtr());
                proc_val->set_type_name(procName.mangled_host_type);
                elabObj->set_field_name(procName.function_name);

                if (isScMethod(qualType)) {
                    proc_val->set_kind(Process::SC_METHOD);
                }
                else if (isScThread(qualType)) {
                    proc_val->set_kind(Process::SC_THREAD);
                }
                else if (isScCThread(qualType)) {
                    proc_val->set_kind(Process::SC_CTHREAD);
                }

            }
            else if (isScSignal(qualType)) {
                elabObj->set_sckind(Object::SC_SIGNAL);
            }
            else if (elabObj->rel_type() != Object::BASE_CLASS) {

                if (isScModularInterface(qualType)) {
                    elabObj->set_sckind(Object::SC_MODULAR_INTERFACE);
                    designDB.add_module_ids(elabObj->id());
                    
                } else if (isScModuleOrInterface(qualType)) {
                    elabObj->set_sckind(Object::SC_MODULE);
                    designDB.add_module_ids(elabObj->id());
                }
            }
            
        } else 
        if (isStdString(qualType)) {
            
            elabObj->set_kind(Object::PRIMITIVE);
            elabObj->mutable_primitive()->set_kind(Primitive::STRING);
            std::string* str_val = elabObj->mutable_primitive()->mutable_str_val();
            *str_val = *reinterpret_cast<const std::string*>(typedObj.getPtr());
        }
    } else {
        sc::ScDiag::reportScDiag(sc::ScDiag::SC_ERROR_ELAB_UNSUPPORTED_TYPE)
                                << typedObj.getType().getAsString();
        SCT_INTERNAL_FATAL_NOLOC ("Design DB generator error");
    }

}
void DesignDbGenerator::fillIntValue(Object *elabObj,
                                   const IntegerObject &intObj) const
{
    auto intVal = intObj.getAPSInt();
    auto initVal = elabObj->mutable_primitive()->mutable_init_val();
    initVal->set_dyn_bitwidth(false);
    initVal->set_bitwidth(intVal.getBitWidth());

    if (intVal.isSigned() && (intVal.getBitWidth() <= 64)) {
        initVal->set_int64_value(intVal.getSExtValue());
    }

    if (intVal.isUnsigned() && (intVal.getBitWidth() <= 64)) {
        initVal->set_uint64_value(intVal.getSExtValue());
    }
}

Object *DesignDbGenerator::createArrayElementObject(TypedObject elTO,
                                                  size_t idx, Object *arrEO)
{
    auto eo = createElabObject(elTO, arrEO->id(), arrEO->is_constant());
    eo->set_rel_type(Object::ARRAY_ELEMENT);
    eo->set_array_idx(idx);
    arrEO->mutable_array()->add_element_ids(eo->id());
    fillElabObject(eo, elTO);

    return eo;
}

void DesignDbGenerator::addModuleDynamicAllocs(TypedObject moduleTO,
                                             Object *moduleEO)
{
    auto &dyn_allocs = get_module_allocs( (sc_core::sc_module*) moduleTO.getPtr());

    for (auto &alloc : dyn_allocs) {

        auto qualType = typeDB.getType(alloc.mangled_type_name);

        if (alloc.is_array) {
            llvm::APInt arraySize(64, alloc.array_size);
            // Use no @SizeExpr, hope that works
            qualType = getAstCtx()->getConstantArrayType(
                            qualType, arraySize, nullptr, ArraySizeModifier::Normal, 0);
        }

        TypedObject dynTO{alloc.ptr, qualType};


        if (!memMap.findElabObj(dynTO)) {
            Object * dynEO = createDynamicElabObject(dynTO, moduleEO);
            if (isAggregate(*dynEO))
                addChildrenRecursive(dynTO, dynEO);
        }

    }
}


Object *DesignDbGenerator::createDynamicElabObject(TypedObject to, Object *moduleEO)
{
    bool isConst = moduleEO->is_constant() ||
        to.getType().isConstQualified();

    auto dynElabObj = createElabObject(to, moduleEO->id(), isConst);

    moduleEO->mutable_record()->add_member_ids(dynElabObj->id());

    fillElabObject(dynElabObj, to);

    dynElabObj->set_rel_type(Object::DYNAMIC);

    return dynElabObj;
}


void DesignDbGenerator::resolvePointers()
{
    using std::cout; using std::endl;
    for (int i = 0; i < designDB.objects_size(); ++i) {
        Object* ptrEO = designDB.mutable_objects(i);

        if (auto ptrTO = getAsPtrOrRefTO(*ptrEO)) {
            auto ptr_val = ptrEO->mutable_primitive()->mutable_ptr_val();

            if (ptrTO->isNullPtr()) {
                ptr_val->set_is_null(true);
                
            } else {
                ptr_val->set_is_null(false);

                if (auto pointeeEO = memMap.resolvePointer(*ptrTO)) {
                    ptr_val->add_pointee_id(pointeeEO->first->id());

                    if (pointeeEO->second) {
                        ptr_val->add_pointee_id(pointeeEO->second);
                    }

                    pointeeEO->first->add_pointer_ids(ptrEO->id());

                    //std::cout << "> resolvePointer : " << ptrEO->id() << " -> " 
                    //           << pointeeEO->first->id() << std::endl;
                } else 
                if (isPort(*ptrEO)) {
                    // If channel binded to port is not found, it means it is outside
                    // of module hierarchy. We create a "virtual" signal in this case

                    TypedObject signalTO = getBindedSignal(*ptrTO, ptrEO);
                    Object* signalEO = createVirtualSignal(signalTO);
                    ptr_val->add_pointee_id(signalEO->id());
                    signalEO->add_pointer_ids(ptrEO->id());
                    //std::cout << "> isPort : " << ptrEO->id() << " -> " << signalEO->id() << std::endl;
                }
            }
        }
    }
}


TypedObject DesignDbGenerator::getBindedSignal(PtrOrRefObject portPtr,
                                               Object* ptrEO) const
{
    while (1) {
        auto pointeeTO = portPtr.dereference().getAs<RecordObject>()
            ->getDynamicTypeObject();

        if (isScBasePort(pointeeTO.getType())) {
            portPtr = getPortBindPtr(pointeeTO);
            if (portPtr.isNullPtr()) {
                ScDiag::reportScDiag(ScDiag::ELAB_PORT_BOUND_PORT_ERROR)
                            << ptrEO->sc_name();
            }
        } else {
            if (!isScSignal(pointeeTO.getType())) {
                ScDiag::reportScDiag(ScDiag::ELAB_PORT_BOUND_SIGNAL_ERROR) 
                            << ptrEO->sc_name();
            }
            return pointeeTO;
        }
    }
}


PtrOrRefObject DesignDbGenerator::getPortBindPtr(TypedObject portTO) const
{
    RecordObject portRec = *portTO.getAs<RecordObject>();

    auto m_interface =
        *portRec.findField("m_interface")->typedObj.getAs<PtrOrRefObject>();

    if (!m_interface.isNullPtr())
        return m_interface;

    auto portBaseTO = portRec.findBase("sc_port_base")->typedObj;

    // for simplicity of elaboration, SystemC kernel was modified here to quickly
    // get pointer to parent port
    void *parentPtr =
        getFirstParentPortPtr((sc_core::sc_port_base *) portBaseTO.getPtr());
    auto parentPtrType = getAstCtx()->getPointerType(getScPortBaseType());

    return *TypedObject(parentPtr, parentPtrType).getAs<PtrOrRefObject>();
}

std::optional<PtrOrRefObject>
DesignDbGenerator::getAsPtrOrRefTO(const sc_elab::Object &eo) const
{
    if (eo.has_primitive()) {
        if (eo.primitive().kind() == Primitive::PORT) {
            return getPortBindPtr(*memMap.findTypedObj(&eo));
        }
        else if (eo.primitive().kind() == Primitive::POINTER) {
            return *memMap.findTypedObj(&eo)->getAs<PtrOrRefObject>();
        }
        else if (eo.primitive().kind() == Primitive::REFERENCE) {
            return *memMap.findTypedObj(&eo)->getAs<PtrOrRefObject>();
        }
    }

    return std::nullopt;

}

void DesignDbGenerator::resolveSensitivity()
{
    for (const Object &eo : designDB.objects()) {

        if (isPort(eo)) {
            resolvePortSensitivity(eo);
        }
        else if (isSignal(eo)) {
            resolveSignalSensitivity(eo);
        }

    }
}


void DesignDbGenerator::resolvePortSensitivity(const Object &portEO)
{
    using std::cout; using std::endl; 
    TypedObject portTO = *memMap.findTypedObj(&portEO);

    auto procs = getSensitiveProcs(
        static_cast<const sc_core::sc_port_base *>(portTO.getPtr()));

    for (auto procSens : procs) {

        auto procBPtrType = getAstCtx()->getPointerType(getScProcessBType());

        TypedObject procPtrTO(&procSens.proc_ptr, procBPtrType);

        if (auto resPtr = memMap.resolvePointer(*procPtrTO.getAs<PtrOrRefObject>())) {
            auto procEO = resPtr->first;
            if (!isProcess(*procEO)) {
                ScDiag::reportScDiag(ScDiag::ELAB_PROCESS_ERROR) 
                                << procEO->sc_name();
            }

            auto *sens = procEO->mutable_primitive()->
                mutable_proc_val()->add_static_events();

            sens->set_event_source_id(portEO.id());

            if (procSens.kind == port_sens_proc::DEFAULT)
                sens->set_kind(Sensitive::DEFAULT);
            else if (procSens.kind == port_sens_proc::POSEDGE)
                sens->set_kind(Sensitive::POSEDGE);
            else if (procSens.kind == port_sens_proc::NEGEDGE)
                sens->set_kind(Sensitive::NEGEDGE);
        } else {
            // See #272 
            cout << "No object for port " << portEO.sc_name() 
                 << ", check sc_module is first in base classes list" << endl;
            assert (false);
        }
    }
}


void DesignDbGenerator::resolveSignalSensitivity(const Object &signalEO)
{
    TypedObject sigTO = *memMap.findTypedObj(&signalEO);
    auto sigRecordObj = *sigTO.getAs<RecordObject>();

    if ( auto negedgeEventP = sigRecordObj.findField("m_negedge_event_p") ) {
        resolveEventSensitivity(negedgeEventP->typedObj, signalEO, Sensitive::NEGEDGE);
    }

    if ( auto posedgeEventP = sigRecordObj.findField("m_posedge_event_p") ) {
        resolveEventSensitivity(posedgeEventP->typedObj, signalEO, Sensitive::POSEDGE);
    }

    if ( auto changeEventP = sigRecordObj.findField("m_change_event_p") ) {
        resolveEventSensitivity(changeEventP->typedObj, signalEO, Sensitive::DEFAULT);
    }

    if ( auto sigResetP = sigRecordObj.findField("m_reset_p") ) {
        resolveSignalReset(sigResetP->typedObj, signalEO);
    }


}


void DesignDbGenerator::resolveEventSensitivity(const TypedObject &eventPtrTO,
                                              const Object &sourceEO,
                                              Sensitive::EventKind kind)
{
    PtrOrRefObject eventPO = *eventPtrTO.getAs<PtrOrRefObject>();

    if (!eventPO.isNullPtr()) {
        auto traverseProcVecFn = [&] (RecordObject::FieldObject &vecField) {
            auto metsArray = *vecField.getAs<ArrayObject>();

            for (size_t i = 0; i < metsArray.size(); ++i) {
                if (auto procPtr = memMap.resolvePointer(
                    *metsArray[i].getAs<PtrOrRefObject>())) {

                    auto procEO = procPtr->first;

                    auto *sens = procEO->mutable_primitive()->
                        mutable_proc_val()->add_static_events();

                    sens->set_event_source_id(sourceEO.id());
                    sens->set_kind(kind);
                }
            }
        };

        auto eventRec = *eventPO.dereference().getAs<RecordObject>();

        if (auto metsStatic = eventRec.findField("m_methods_static")) {
            traverseProcVecFn(*metsStatic);
        }

        if (auto threadsStatic = eventRec.findField("m_threads_static")) {
            traverseProcVecFn(*threadsStatic);
        }
    }
}


void DesignDbGenerator::resolveSignalReset(const TypedObject &resetPtrTO,
                                         const Object &sourceEO)
{
    auto sigResetPtrObj = *resetPtrTO.getAs<PtrOrRefObject>();

    if (!sigResetPtrObj.isNullPtr()) {

        auto resetRO = *sigResetPtrObj.dereference().getAs<RecordObject>();

        auto targetsAO = *resetRO.findField("m_targets")->getAs<ArrayObject>();

        for (auto target : targetsAO) {

            auto resetTargetTO = *target.getAs<RecordObject>();

            bool async = resetTargetTO.findField("m_async")->getAs<IntegerObject>()
                ->getAPSInt().getBoolValue();
            bool level = resetTargetTO.findField("m_level")->getAs<IntegerObject>()
                ->getAPSInt().getBoolValue();

            auto procPO = *resetTargetTO.findField("m_process_p")->getAs<PtrOrRefObject>();
            
            if (auto procVal = memMap.resolvePointer(procPO)) {
                if (auto procEO = procVal->first) {
                    auto *reset = procEO->mutable_primitive()->
                        mutable_proc_val()->add_resets();

                    reset->set_source_id(sourceEO.id());
                    reset->set_async(async);
                    reset->set_level(level);
                }
            } else {
                // No process for reset signal, that is possible when signal
                // connected to sc_in<bool> and processes sensitive to that
            }
        }
    }
}

void DesignDbGenerator::resolveResetFinders()
{
    auto simCtxType = getScSimContextType();
    auto *simCtrPtr = getScSimContext();
    auto simCtxRO = *TypedObject(simCtrPtr, simCtxType).getAs<RecordObject>();

    auto resetFinderP = *simCtxRO.findField("m_reset_finder_q")->getAs<PtrOrRefObject>();

    while (!resetFinderP.isNullPtr()) {

        auto resetFinderRO = *resetFinderP.dereference().getAs<RecordObject>();

        bool async = resetFinderRO.findField("m_async")->getAs<IntegerObject>()
                     ->getAPSInt().getBoolValue();

        bool level = resetFinderRO.findField("m_level")->getAs<IntegerObject>()
            ->getAPSInt().getBoolValue();

        resetFinderP = *resetFinderRO.findField("m_next_p")->getAs<PtrOrRefObject>();

        auto m_target_p = *resetFinderRO.findField("m_target_p")->getAs<PtrOrRefObject>();

        if (auto procPtr = memMap.resolvePointer(m_target_p)) {
            Object * targetProcEO = procPtr->first;

            auto m_in_p = *resetFinderRO.findField("m_in_p")->getAs<PtrOrRefObject>();
            auto m_inout_p = *resetFinderRO.findField("m_in_p")->getAs<PtrOrRefObject>();
            auto m_out_p = *resetFinderRO.findField("m_in_p")->getAs<PtrOrRefObject>();

            Object *sourcePort = nullptr;

            if (!m_in_p.isNullPtr()) {
                sourcePort = memMap.resolvePointer(m_in_p)->first;
            } else if (!m_inout_p.isNullPtr()) {
                sourcePort = memMap.resolvePointer(m_inout_p)->first;
            } else if (!m_out_p.isNullPtr()) {
                sourcePort = memMap.resolvePointer(m_out_p)->first;
            }

            auto *newReset = targetProcEO->mutable_primitive()->
                mutable_proc_val()->add_resets();

            newReset->set_async(async);
            newReset->set_level(level);
            newReset->set_source_id(sourcePort->id());
        }
    }

}

Object *DesignDbGenerator::createVirtualSignal(TypedObject signalTO)
{
    Object * signalEO = createElabObject(signalTO, 0, false);
    fillElabObject(signalEO, signalTO);
    signalEO->set_rel_type(Object::NO_PARENT);

    addChildrenRecursive(signalTO, signalEO);

    return signalEO;
}

} // namespace sc_elab

