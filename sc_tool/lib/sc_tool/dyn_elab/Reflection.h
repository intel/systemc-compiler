/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_REFLECTION_H
#define SCTOOL_REFLECTION_H

#include <clang/AST/Type.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclCXX.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/APSInt.h>
#include <optional>
#include <type_traits>
#include <unordered_map>

// Clang-based C++/SystemC Reflection
// Requires patched SystemC library for dynamic allocations

namespace sc_elab
{

/// TypedObject packs together pointer to object in memory and its QualType
class TypedObject
{
    // Real pointer to object in memory after elaboration
    const void * objPtr = nullptr;
    clang::QualType canonType;

public:

    TypedObject(const void *objPtr, clang::QualType type);
    TypedObject() = default;

    bool operator == (const TypedObject &other) const;
    bool operator != (const TypedObject &other) const;

    clang::QualType getType() const;
    const void * getPtr() const;

    size_t getSizeInBytes() const;

    bool isRecordObject() const;
    bool isIntegerObject() const;
    bool isArrayObject() const;
    bool isScObject() const;
    bool isPtrOrRefObject() const;

    /// Array or Record
    bool isAggregate() const;

    template <typename T>
    bool isKindOf() const;

    /// Get specific wrapper, possible Ts are:
    /// RecordObject, ScModuleObject, ArrayObject
    template <typename T>
    std::optional<T> getAs() const;

    /// Remove cv qualifiers
    TypedObject getUnqualified() const;

    template <typename OS>
    friend OS & operator << (OS & os, const TypedObject &obj);
};

class RecordObject
{
public:

    template <typename T>
    struct RecordItemObject {
        TypedObject typedObj;
        T itemDecl;

        template <typename U>
        std::optional<U> getAs() const { return typedObj.getAs<U>(); }
    };

    using BaseClassObject = RecordItemObject<clang::CXXBaseSpecifier>;
    using FieldObject = RecordItemObject<clang::FieldDecl*>;

    template<typename ItemT, typename InnerIteratorT>
    class RecordItemIterator {
    public:

        using value_type = ItemT;

        RecordItemIterator() = default;
        RecordItemIterator(const RecordObject * recObj, InnerIteratorT innerIter);

        RecordItemIterator& operator++();    // prefix increment
        RecordItemIterator operator++(int);  // postfix increment
        ItemT operator*() const;
        bool operator==(const RecordItemIterator &other) const;
        bool operator!=(const RecordItemIterator &other) const;

    private:
        const RecordObject *recordObject;
        InnerIteratorT innerIterator;
    };

    using FieldIterator = RecordItemIterator<FieldObject,
                                clang::CXXRecordDecl::field_iterator >;

    using BasesIterator = RecordItemIterator<BaseClassObject,
                                 clang::CXXRecordDecl::base_class_const_iterator >;

    using FieldRange = llvm::iterator_range<FieldIterator>;
    using BasesRange = llvm::iterator_range<BasesIterator>;

    const clang::CXXRecordDecl * getRecordDecl() const;

    void dump() const;
    void dumpDynamicType() const;

    /// get dynamic type of polymorphic object
    bool isPolymorphic() const;
    clang::QualType getDynamicType() const;
    TypedObject getDynamicTypeObject() const;

    FieldIterator field_begin() const;
    FieldIterator field_end() const;
    FieldRange fields() const;

    BasesIterator bases_begin() const;
    BasesIterator bases_end() const;
    BasesRange bases() const;

    BasesIterator vbases_begin() const;
    BasesIterator vbases_end() const;
    BasesRange vbases() const;

    // return first found field with given name
    std::optional<FieldObject> findField(llvm::StringRef fieldName) const;

    // return first base with given name
    std::optional<BaseClassObject> findBase(llvm::StringRef baseName) const;

    template <typename OS>
    friend OS & operator << (OS & os, const RecordObject &obj);

protected:
    /// created from TypedObject
    RecordObject(TypedObject typedObj);
    const TypedObject typedObj;

private:
    friend class TypedObject;

    // get offset to start of object for polymorphic objects
    ptrdiff_t getOffsetToTop() const;
};

template<>
RecordObject::FieldIterator::value_type
RecordObject::FieldIterator::operator*() const;

template<>
RecordObject::BasesIterator::value_type
RecordObject::BasesIterator::operator*() const;


class ScObject : public RecordObject {
public:
    ScObject(TypedObject typedObj);

    bool isPort() const;
    bool isInPort() const;
    bool isOutPort() const;
    bool isInOutPort() const;
    bool isSignal() const;
    bool isModule() const;
    bool isModularInterface() const;

    const std::string & scName() const;
};

class IntegerObject
{
    friend class TypedObject;
    TypedObject typedObj;
    /// created from TypedObject
    IntegerObject(TypedObject typedObj);

public:

    llvm::APSInt getAPSInt() const;

    template <typename OS>
    friend OS & operator << (OS & os, const IntegerObject &obj);
};


class ArrayObject
{
public:

    enum KIND {
        C_ARRAY, STD_VECTOR, STD_ARRAY, SC_VECTOR
    };

    clang::QualType getType() const;
    const void * getPtr() const;

    KIND getKind() const;
    size_t size() const;
    TypedObject operator[] (size_t idx) const;
    clang::QualType getElementType() const;

    /// for arrays of arrays, returns most inner (non array) element type
    clang::QualType getInnerElementType() const;
    /// for arrays of arrays, returns most inner (non array) element
    TypedObject getFirstInnerElement () const;

    /// get dimensions of multidimensional array (array of arrays)
    std::vector<size_t> dimensions() const;

    struct ArrayObjectIterator {
        using value_type = TypedObject;

        ArrayObjectIterator() = default;
        ArrayObjectIterator(const ArrayObject &ao, size_t idx);

        ArrayObjectIterator& operator++();    // prefix increment
        ArrayObjectIterator operator++(int);  // postfix increment
        TypedObject operator*() const;
        bool operator==(const ArrayObjectIterator &other) const;
        bool operator!=(const ArrayObjectIterator &other) const;

    private:
        const ArrayObject &ao;
        size_t idx;
    };

    ArrayObjectIterator begin() const;
    ArrayObjectIterator end() const;

    bool operator==(const ArrayObject &other) const;

    static std::optional<KIND> getArrayObjectKind(clang::QualType canonicalType);

    template <typename OS>
    friend OS & operator << (OS & os, const ArrayObject &obj);

private:

    friend class TypedObject;

    TypedObject typedObj;
    KIND kind;

    /// created from TypedObject
    ArrayObject(TypedObject typedObj);
};

class PtrOrRefObject
{
    friend class TypedObject;
    TypedObject typedObj;

public:
    PtrOrRefObject(TypedObject typedObj);

    bool isPointer() const;
    bool isReference() const;
    bool isNullPtr() const;
    clang::QualType getPointeeType() const;
    TypedObject dereference() const;

    template <typename OS>
    friend OS & operator << (OS & os, const PtrOrRefObject &obj);
};


inline llvm::hash_code hash_value(const TypedObject & obj)
{
    return llvm::hash_combine(obj.getType().getAsOpaquePtr(), obj.getPtr());
}


// -----------------------------------------------------------------------------
// Implementation

inline TypedObject::TypedObject(const void *objPtr, clang::QualType type)
: objPtr(objPtr), canonType(type.getCanonicalType())
{}

inline bool TypedObject::operator==(const TypedObject &other) const
{
    return (objPtr == other.objPtr) && (canonType == other.canonType);
}

inline bool TypedObject::operator!=(const TypedObject &other) const
{
    return !(*this == other);
}

inline clang::QualType TypedObject::getType() const
{
    return canonType;
}

inline const void *TypedObject::getPtr() const
{
    return objPtr;
}

template<typename T>
bool TypedObject::isKindOf() const
{
    if (std::is_same<T, ScObject>::value)
        return isScObject();

    if (std::is_same<T, RecordObject>::value)
        return isRecordObject();

    if (std::is_same<T, IntegerObject>::value)
        return isIntegerObject();

    if (std::is_same<T, ArrayObject>::value)
        return isArrayObject();

    if (std::is_same<T, PtrOrRefObject>::value)
        return isPtrOrRefObject();

    return false;
}

inline TypedObject TypedObject::getUnqualified() const
{
    return TypedObject(getPtr(),getType().getUnqualifiedType());
}


template<typename T>
inline std::optional<T> TypedObject::getAs() const
{
    if (isKindOf<T>())
        return T(*this);

    return std::nullopt;
}

template<typename OS> OS &operator<<(OS &os, const TypedObject &obj)
{
    os << "OBJ @" << obj.getPtr()
       << " Size: " << obj.getSizeInBytes() << " :";

    if (auto intObj = obj.getAs<IntegerObject>())
        os << *intObj;
    else if (auto arrayObj = obj.getAs<ArrayObject>())
        os << *arrayObj;
    else if (auto modObj = obj.getAs<ScObject>())
        os << *modObj;
    else if (auto recordObj = obj.getAs<RecordObject>())
        os << *recordObj;
    else if (auto ptrOrRefObj = obj.getAs<PtrOrRefObject>())
        os << *ptrOrRefObj;

    return os;
}


inline RecordObject::RecordObject(TypedObject typedObj)
: typedObj(typedObj)
{}

inline const clang::CXXRecordDecl *RecordObject::getRecordDecl() const
{
    return typedObj.getType()->getAsCXXRecordDecl();
}

template<typename ItemT, typename InnerIteratorT>
RecordObject::RecordItemIterator<ItemT, InnerIteratorT>::RecordItemIterator(
    const RecordObject *recObj, InnerIteratorT innerIter)
    : recordObject(recObj), innerIterator(innerIter)
{
}

template<typename ItemT, typename InnerIteratorT>
RecordObject::RecordItemIterator<ItemT, InnerIteratorT> &
RecordObject::RecordItemIterator<ItemT, InnerIteratorT>::operator++()
{
    ++innerIterator;
    return *this;
}

template<typename ItemT, typename InnerIteratorT>
RecordObject::RecordItemIterator<ItemT, InnerIteratorT>
RecordObject::RecordItemIterator<ItemT, InnerIteratorT>::operator++(int)
{
    auto oldIter = *this;
    this->operator++();
    return oldIter;
}

template<typename ItemT, typename InnerIteratorT>
bool RecordObject::RecordItemIterator<ItemT, InnerIteratorT>::operator==(
    const RecordObject::RecordItemIterator<ItemT, InnerIteratorT> &other) const
{
    return (recordObject == other.recordObject) &&
        (innerIterator == other.innerIterator);
}

template<typename ItemT, typename InnerIteratorT>
bool RecordObject::RecordItemIterator<ItemT, InnerIteratorT>::operator!=(
    const RecordObject::RecordItemIterator<ItemT, InnerIteratorT> &other) const
{
    return !(*this == other);
}

template<typename OS>
OS &operator<<(OS &os, const RecordObject &obj)
{
    os << "RecordObject: " << obj.getRecordDecl()->getName() << "\n";

    for (auto field : obj.fields()) {
        os << "Field " << field.itemDecl->getName() << " " << field.typedObj << "\n";
    }

    for (auto base : obj.bases() ) {
        auto baseRecordObj = base.getAs<RecordObject>();
        os << "Base " << baseRecordObj->getRecordDecl()->getName() << "\n";
        os << *baseRecordObj;
    }

    return os;
}


inline IntegerObject::IntegerObject(TypedObject typedObj)
: typedObj(typedObj)
{
}

template<typename OS>
OS &operator<<(OS &os, const IntegerObject &obj)
{
    os << obj.getAPSInt();
    return os;
}

template<typename OS>
OS &operator<<(OS &os, const ArrayObject &obj)
{
    os << "array size :" << obj.size() << "\n";
    os << "ElementType : " << obj.getElementType().getAsString() << "\n";
    os << "elements: ";

    for (size_t i = 0; i < obj.size(); ++i) {
        os << obj[i] << " ";
    }
    os << "\n";

    return os;
}


inline PtrOrRefObject::PtrOrRefObject(TypedObject typedObj)
: typedObj(typedObj)
{
    assert(typedObj.getType()->isPointerType()
    || typedObj.getType()->isReferenceType());
}

inline bool PtrOrRefObject::isPointer() const
{
    return typedObj.getType()->isPointerType();
}

inline bool PtrOrRefObject::isReference() const
{
    return typedObj.getType()->isReferenceType();
}

inline bool PtrOrRefObject::isNullPtr() const
{
    if (typedObj.getPtr() == nullptr) {
        return true;
    }
    const void * pVal = *reinterpret_cast<const void * const *>(typedObj.getPtr());
    return pVal == nullptr;
}

inline clang::QualType PtrOrRefObject::getPointeeType() const
{
    return typedObj.getType()->getPointeeType();
}

inline TypedObject PtrOrRefObject::dereference() const
{
    assert(!isNullPtr());
    const void * pVal = *reinterpret_cast<const void * const *>(typedObj.getPtr());
    return TypedObject(pVal, getPointeeType());
}

template<typename OS>
OS &operator<<(OS &os, const PtrOrRefObject &obj)
{
    if (obj.isPointer()) {
        os << "Pointer ";
    } else if (obj.isReference()) {
        os << "Reference ";
    }

    if (obj.isNullPtr()) {
        os << " isNull";
    } else {
        os << (uintptr_t) obj.typedObj.getPtr();
    }

    return os;
}

} // namespace sc_elab


namespace std
{

/// TypedObject hashing support for std::set and std::map
template <>
struct hash<sc_elab::TypedObject>
{
    std::size_t operator()(const sc_elab::TypedObject &obj) const
    {
        return hash_value(obj);
    }
};

} // namespace std


#endif //SCTOOL_REFLECTION_H
