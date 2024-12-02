/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_ELAB_DBGENERATOR_H
#define SCTOOL_ELAB_DBGENERATOR_H

#include <sc_elab.pb.h>

#include <sc_tool/dyn_elab/MangledTypeDB.h>
#include <sc_tool/dyn_elab/Reflection.h>
#include <sc_tool/dyn_elab/GlobalContext.h>
#include <sc_tool/dyn_elab/ElabTypeManager.h>
#include <sc_tool/utils/ScTypeTraits.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/TypeOrdering.h>

#include <vector>

/**
 *
 * This is a port of old Python elaborator in SystemC, without any design
 * improvements. It creates a protobuf-based Elaboration DB.
 *
 * TODO: Get rid of protobuf, design a clear C++ API for elaboration
 *
 * naming conventions:
 * eo - Elaboration object from protobuf
 * to - Typed object (Clang-based reflection)
 * ro - RecordObject (Clang-based reflection)
 * po - PtrOrRefObject (Clang-based reflection)
 * ao - ArrayObject (Clang-based reflection)
 * */
namespace sc_elab
{

/// Maps between reflection objects and protobuf objects
class ObjectMap {
public:

    void addObject(TypedObject to, Object *o);

    Object * findElabObj (TypedObject to);
    std::optional<TypedObject> findTypedObj (const Object *eo) const;

    /// Find an object a pointer points to by analyzing an address (value) and type
    /// of pointer
    /// returns object + offset (index in array)
    std::optional<std::pair<Object *, size_t>> resolvePointer (PtrOrRefObject ptrObj);

private:

    std::unordered_map<TypedObject, Object *> rfl2ElabMap;
    std::unordered_map<const Object *, TypedObject> elab2RflMap;
    std::vector<TypedObject> allTOs;
};

class DesignDbGenerator
{
    SCDesign &designDB;
    ElabTypeManager &typeManager;
    ObjectMap memMap;
    MangledTypeDB &typeDB = *getMangledTypeDB();

public:
    typedef uint32_t ID;

    DesignDbGenerator(SCDesign &designDB, ElabTypeManager &typeManager)
    : designDB(designDB), typeManager(typeManager)
    {}

    void run(clang::CXXRecordDecl *modRecordDecl, void *objectPtr);

private:

    /// Create EO and add it to memory map
    /// \param to - reflection object
    /// \param parentID - ID of parent EO
    /// \param isConst - object is constant
    /// \return - pointer to created EO
    Object* createElabObject(TypedObject to, ID parentID, bool isConst);

    /// Create a "field" EO (data member or base class)
    /// \param fieldObj - field reflection object
    /// \param parentElabObj - parent object (always a record)
    /// \param isBaseClass - true if object is base class
    /// \return - pointer to created EO
    template <class FieldT>
    Object* createFieldElabObject(FieldT fieldObj, Object* parentElabObj
        , bool isBaseClass);

    /// Create dynamically allocated EO
    /// \param to  - reflection object
    /// \param moduleEO - EO of module where allocation was made
    /// \return - pointer to created EO
    Object* createDynamicElabObject(TypedObject to, Object *moduleEO);

    /// Create array element EO
    /// \param elTO - reflection object of array element
    /// \param idx - index of element
    /// \param arrEO - parrent array EO
    /// \return - pointer to created EO
    Object* createArrayElementObject(TypedObject elTO,
                                     size_t idx, Object* arrEO);

    /// Fill EO properties based on its type and value
    void fillElabObject(Object* elabObj, TypedObject typedObj);

    /**
     * Recursively add all children to design database
     * @param hostTO - reflected host object in memory
     * @param hostEO - host protobuf object
     */
    void addChildrenRecursive(TypedObject hostTO, Object *hostEO);

    /// Create elaboration objects for dynamically-allocated objects
    void addModuleDynamicAllocs(TypedObject moduleTO, Object *moduleEO);

    /// Fill integer value and bitwidth
    void fillIntValue(Object *elabObj, const IntegerObject &intObj) const;

    /// Fill pointees for pointer and pointer-like objects
    void resolvePointers();

    TypedObject getBindedSignal(PtrOrRefObject portPtr, Object* ptrEO) const;

    /// Create a "virtual" signal outside of module hierarchy
    Object* createVirtualSignal(TypedObject signalTO);

    void resolveSensitivity();
    void resolvePortSensitivity(const Object &portEO);
    void resolveSignalSensitivity(const Object &signalEO);
    void resolveEventSensitivity(const TypedObject &eventPtrTO,
        const Object &sourceEO, Sensitive::EventKind kind);

    void resolveSignalReset(const TypedObject &resetPtrTO, const Object &sourceEO);

    void resolveResetFinders();

    /// If EO is pointer-like object, get corresponding pointer TO
    std::optional<PtrOrRefObject> getAsPtrOrRefTO(const sc_elab::Object &eo) const;

    /// Get pointer to binded channel from sc_port
    PtrOrRefObject getPortBindPtr(TypedObject portTO) const;
};


}
#endif //SCTOOL_ELAB_DBGENERATOR_H
