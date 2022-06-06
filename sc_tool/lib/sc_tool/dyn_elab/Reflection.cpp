/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */


#include <sc_tool/utils/CppTypeTraits.h>
#include <sc_tool/dyn_elab/Reflection.h>
#include <sc_tool/dyn_elab/GlobalContext.h>

#include <clang/AST/RecordLayout.h>
#include <clang/AST/VTableBuilder.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <llvm/Support/raw_ostream.h>
#include <sc_tool/utils/ScTypeTraits.h>

#include <rtti_sysc/SystemCRTTI.h>

#include <type_traits>
#include <iostream>

#include "sc_tool/diag/ScToolDiagnostic.h"

using namespace clang;
using namespace sc;

namespace sc_elab
{

static size_t sizeOf(QualType objType) {
    return getAstCtx()->getTypeSize(objType) / 8;
}


// class TypedObject -----------------------------------------------------------

bool TypedObject::isRecordObject() const
{
    return getType()->getCanonicalTypeInternal()->isRecordType();
}

bool TypedObject::isIntegerObject() const
{
    return isAnyInteger(canonType);
}

bool TypedObject::isArrayObject() const
{
    auto canonicalType = getType().getCanonicalType();

    if (ArrayObject::getArrayObjectKind(canonicalType))
        return true;

    return false;
}

bool TypedObject::isPtrOrRefObject() const
{
    return getType()->isPointerType() || getType()->isReferenceType();
}

size_t TypedObject::getSizeInBytes() const
{
    return sizeOf(getType());
}

bool TypedObject::isScObject() const
{
    return sc::isScObject(getType());
}

bool TypedObject::isAggregate() const
{
    return isRecordObject() || isArrayObject();
}


// class RecordObject ----------------------------------------------------------

void RecordObject::dump() const
{
    llvm::outs() << "RecordObject DUMP\n";

    llvm::outs() << this->typedObj << "\n";

    std::vector<int> ivec;
    ivec.data();
}

void RecordObject::dumpDynamicType() const
{

    for (auto field: fields()) {
        if (auto intObj = field.typedObj.getAs<IntegerObject>()) {
            llvm::outs() << field.itemDecl->getName() << " : " << intObj->getAPSInt() << "\n";
        }
    }

    if (auto foundField = findField("x00")) {
        llvm::outs() << "field x00 " << foundField->typedObj.getAs<IntegerObject>()->getAPSInt() << "\n";
    }

    if (!getRecordDecl()->isDynamicClass())
        return;

    llvm::outs() << "VBases!:\n";
    for (auto base : vbases()) {
        llvm::outs() << "   vbase: " <<  base.typedObj.getType().getAsString() << "\n";
        base.typedObj.getAs<RecordObject>()->dumpDynamicType();
    }

    auto mangledName = getDynamicMangledTypeName(this->typedObj.getPtr());
    llvm::outs() << "dumpDynamicType: " <<  mangledName << "\n";
    llvm::outs() << getMangledTypeDB()->getType(mangledName).getAsString() << "\n";

    for (auto base : bases()) {
        llvm::outs() << "   base: " <<  base.typedObj.getType().getAsString() << "\n";
        base.typedObj.getAs<RecordObject>()->dumpDynamicType();
    }
}

bool RecordObject::isPolymorphic() const
{
    const clang::CXXRecordDecl* recDecl = getRecordDecl();
    return (recDecl && recDecl->hasDefinition() && recDecl->isPolymorphic());
}

clang::QualType RecordObject::getDynamicType() const
{
    if (!getRecordDecl()->isDynamicClass())
        return this->typedObj.getType();

    auto mangledName = getDynamicMangledTypeName(this->typedObj.getPtr());
    return getMangledTypeDB()->getType(mangledName);
}

TypedObject RecordObject::getDynamicTypeObject() const
{
    if (!getRecordDecl()->isDynamicClass())
        return this->typedObj;

    auto dynType = getDynamicType();

    if (dynType == this->typedObj.getType())
        return this->typedObj;

    const void *dynObjPtr = (const char *)(this->typedObj.getPtr()) + getOffsetToTop();

    return TypedObject{dynObjPtr, dynType};
}

RecordObject::FieldIterator RecordObject::field_begin() const
{
    return RecordObject::FieldIterator(this, getRecordDecl()->field_begin());
}

RecordObject::FieldIterator RecordObject::field_end() const
{
    return RecordObject::FieldIterator(this, getRecordDecl()->field_end());
}

RecordObject::FieldRange RecordObject::fields() const
{
    return FieldRange(field_begin(), field_end());
}

RecordObject::BasesIterator RecordObject::bases_begin() const
{
    return BasesIterator(this, getRecordDecl()->bases_begin());
}

RecordObject::BasesIterator RecordObject::bases_end() const
{
    return BasesIterator(this, getRecordDecl()->bases_end());
}

RecordObject::BasesRange RecordObject::bases() const
{
    return BasesRange(bases_begin(), bases_end());
}

RecordObject::BasesIterator RecordObject::vbases_begin() const
{
    return BasesIterator(this, getRecordDecl()->vbases_begin());
}

RecordObject::BasesIterator RecordObject::vbases_end() const
{
    return BasesIterator(this, getRecordDecl()->vbases_end());
}

RecordObject::BasesRange RecordObject::vbases() const
{
    return BasesRange(vbases_begin(), vbases_end());
}

ptrdiff_t RecordObject::getOffsetToTop() const
{
    if (!getRecordDecl()->isDynamicClass())
        return 0;

    // TODO: MSVC ABI
    const ptrdiff_t **vptr = (const ptrdiff_t **)(this->typedObj.getPtr());

    // Itanium ABI VTable
    // [-2]  Offset to top
    // [-1]  RTTI
    // [0..] Virtual functions
    return (*vptr)[-2];
}

Optional<RecordObject::FieldObject>
RecordObject::findField(llvm::StringRef fieldName) const
{
    for (auto field : fields()) {
        if (fieldName == field.itemDecl->getName())
            return field;
    }

    for (auto base : bases()) {
        if (auto foundField = base.getAs<RecordObject>()->findField(fieldName)) {
            return foundField;
        }
    }

    return llvm::None;
}

llvm::Optional<RecordObject::BaseClassObject>
RecordObject::findBase(llvm::StringRef baseName) const {
    for (auto base : bases()) {
        //llvm::outs() << "found_base " << base.itemDecl.getType()->getAsCXXRecordDecl()->getName() << "\n";

        if (baseName == base.itemDecl.getType()->getAsCXXRecordDecl()->getName())
            return base;
    }

    for (auto base : bases()) {
        if (auto foundBase = base.getAs<RecordObject>()->findBase(baseName)) {
            return foundBase;
        }
    }

    return llvm::None;
}


template<>
RecordObject::FieldIterator::value_type
RecordObject::FieldIterator::operator*() const {
    auto * recDecl = this->recordObject->getRecordDecl();
    auto &recordLayout = getAstCtx()->getASTRecordLayout(recDecl);
    auto *fieldDecl = *(this->innerIterator);
    size_t offsetBits = recordLayout.getFieldOffset(fieldDecl->getFieldIndex());
    size_t offsetBytes = offsetBits / 8;

    const void * objPtr = this->recordObject->typedObj.getPtr();
    const void * fieldPtr = (char*)objPtr + offsetBytes;
    return FieldObject{TypedObject{fieldPtr, fieldDecl->getType()}, fieldDecl};
}

template<>
RecordObject::BasesIterator::value_type
RecordObject::BasesIterator::operator*() const {
    auto * recDecl = this->recordObject->getRecordDecl();
    auto baseSpec = *(this->innerIterator);
    auto *baseDecl = baseSpec.getType()->getAsCXXRecordDecl();

    ptrdiff_t baseOffset;
    if (baseSpec.isVirtual()) {
        auto dynObj = this->recordObject->getDynamicTypeObject();
        const void * objPtr = dynObj.getPtr();
        auto &recordLayout = getAstCtx()->getASTRecordLayout(
            dynObj.getAs<RecordObject>()->getRecordDecl());

        if (recordLayout.getVBaseOffsetsMap().count(baseDecl) == 0) {
            ScDiag::reportScDiag(ScDiag::ELAB_BAD_RECORD_OBJ);
        }
        baseOffset = recordLayout.getVBaseClassOffset(baseDecl).getQuantity();

        const void *basePtr = ((const char*)objPtr + baseOffset);

        return BaseClassObject{TypedObject{basePtr, baseSpec.getType()}, baseSpec};

    } else {
        auto &recordLayout = getAstCtx()->getASTRecordLayout(recDecl);
        baseOffset = recordLayout.getBaseClassOffset(baseDecl).getQuantity();
        const void * objPtr = this->recordObject->typedObj.getPtr();
        const void *basePtr = ((const char*)objPtr + baseOffset);

        return BaseClassObject{TypedObject{basePtr, baseSpec.getType()}, baseSpec};

    }

}


// class IntegerObject ---------------------------------------------------------
template <typename T>
static llvm::APSInt getPtrAs(const void* objPtr) {
    llvm::APSInt res(sizeof(T) * 8, std::is_unsigned<T>::value);
    res = *static_cast<const T*>(objPtr);
    return res;
}

static llvm::APSInt getBuiltinInt(const void* objPtr, const BuiltinType *buitinType) {
    switch(buitinType->getKind()) {

        case BuiltinType::Kind::SChar:
            return getPtrAs<signed char>(objPtr);

        case BuiltinType::Kind::Char_S:
            return getPtrAs<signed char>(objPtr);

        case BuiltinType::Kind::Short:
            return getPtrAs<short>(objPtr);

        case BuiltinType::Kind::Int:
            return getPtrAs<int>(objPtr);

        case BuiltinType::Kind::Long:
            return getPtrAs<long>(objPtr);

        case BuiltinType::Kind::LongLong:
            return getPtrAs<long long>(objPtr);

        case BuiltinType::Kind::Bool: {
            llvm::APSInt res(1, true);
            res = *static_cast<const bool*>(objPtr);
            return res;
        }

        case BuiltinType::Kind::UChar:
            return getPtrAs<unsigned char>(objPtr);

        case BuiltinType::Kind::Char_U:
            return getPtrAs<unsigned char>(objPtr);

        case BuiltinType::Kind::UShort:
            return getPtrAs<unsigned short>(objPtr);

        case BuiltinType::Kind::UInt:
            return getPtrAs<unsigned int>(objPtr);

        case BuiltinType::Kind::ULong:
            return getPtrAs<unsigned long>(objPtr);

        case BuiltinType::Kind::ULongLong:
            return getPtrAs<unsigned long long>(objPtr);

        default:
            assert(false);
            return llvm::APSInt::get(0);
    }

}

llvm::APSInt IntegerObject::getAPSInt() const
{
    QualType thisType = typedObj.getType();
    auto* objPtr = typedObj.getPtr();

    if (thisType->isBuiltinType()) {
        auto builtinType = typedObj.getType()->getAs<BuiltinType>();
        return getBuiltinInt(objPtr, builtinType);
    }
    else if (thisType->isEnumeralType()) {
        auto *enumType = thisType->getAs<EnumType>();
        auto builtinType = enumType->getDecl()->getIntegerType()->getAs<BuiltinType>();
        return getBuiltinInt(objPtr, builtinType);
    }
    else if (thisType->isRecordType()) {

        if (isScInt(thisType)) {
            return getScIntValue(objPtr);
        }
        else if (isScUInt(thisType)) {
            return getScUIntValue(objPtr);
        }
        else if (isScBigInt(thisType) ) {
            return getScBigIntValue(objPtr);
        }
        else if (isScBigUInt(thisType)) {
            return getScBigUIntValue(objPtr);
        }
        else if (isScBitVector(thisType)) {
            return getScBitVectorValue(objPtr);
        }
    }

    assert(false);
    return llvm::APSInt::get(0);
}

// class ArrayObject  ----------------------------------------------------------

llvm::Optional<ArrayObject::KIND>
ArrayObject::getArrayObjectKind(clang::QualType canonicalType)
{
    if (canonicalType->isConstantArrayType()) {

        return ArrayObject::C_ARRAY;

    } else if (canonicalType->getAsCXXRecordDecl()) {

        if (isScVector(canonicalType))
            return ArrayObject::SC_VECTOR;

        if (isStdVector(canonicalType))
            return ArrayObject::STD_VECTOR;
        
        if (isStdArray(canonicalType))
            return ArrayObject::STD_ARRAY;
    }

    return llvm::None;
}

ArrayObject::ArrayObject(sc_elab::TypedObject typedObj)
    : typedObj(typedObj)
{
    kind = *getArrayObjectKind(typedObj.getType().getCanonicalType());
}

static std::vector<void*> * getScVecInner(const TypedObject& scVecObj) {
    auto innerVecField = *scVecObj.getAs<RecordObject>()->findField("vec_");
    return (std::vector<void*> *)innerVecField.typedObj.getPtr();
}


size_t ArrayObject::size() const
{
    switch (kind) {
        case C_ARRAY: {
            auto type = typedObj.getType();
            auto constArrayType = dyn_cast<clang::ConstantArrayType>(type);
            return constArrayType->getSize().getZExtValue();
        }

        case STD_VECTOR: {
            auto elSize = sizeOf(getElementType());

            // This is implementation-specific, needs to be tested on different platforms
            std::vector<uint8_t> *byteVec =(std::vector<uint8_t> *) typedObj.getPtr();

            return byteVec->size() / elSize;
        }

        case STD_ARRAY: {
            auto type = typedObj.getType();
            auto tempArg = sc::getTemplateArg(type, 1);
            size_t size = tempArg->getAsIntegral().getZExtValue();
            
            return size;
        }

        case SC_VECTOR: {
            return getScVecInner(typedObj)->size();
        }
    }

    return 0;
}

ArrayObject::KIND ArrayObject::getKind() const
{
    return kind;
}

TypedObject ArrayObject::operator[](size_t idx) const
{
    auto elementType = getElementType();
    auto sizeOfEl = sizeOf(elementType);

    uint8_t *startPtr = nullptr;

    void * elPtr;

    switch (kind) {
        case C_ARRAY:
            startPtr = (uint8_t *)typedObj.getPtr();
            elPtr = startPtr + idx * sizeOfEl;
            break;

        case STD_VECTOR: {
            std::vector<uint8_t> *byteVec =(std::vector<uint8_t> *) typedObj.getPtr();
            startPtr = byteVec->data();
            elPtr = startPtr + idx * sizeOfEl;
        } break;
        
        case STD_ARRAY: {
            std::array<uint8_t, 1> *byteVec =(std::array<uint8_t,1>*) typedObj.getPtr();
            startPtr = byteVec->data();
            elPtr = startPtr + idx * sizeOfEl;
        } break;

        case SC_VECTOR:
            elPtr = getScVecInner(typedObj)->at(idx);
            break;
    }


    return TypedObject(elPtr, elementType);
}

clang::QualType ArrayObject::getElementType() const
{
    auto type = typedObj.getType();

    switch (kind) {
        case C_ARRAY: {
            auto constArrayType = dyn_cast<clang::ConstantArrayType>(type);
            return constArrayType->getElementType();
        }

        default: // SC_VECTOR, STD_VECTOR
        {
            auto recDecl = type->getAsCXXRecordDecl();
            auto tempSpecDecl = dyn_cast<clang::ClassTemplateSpecializationDecl>(recDecl);
            return tempSpecDecl->getTemplateArgs().get(0).getAsType();
        }
    }
}

clang::QualType ArrayObject::getInnerElementType() const
{
    assert (size() != 0 && "Zero-size array/vector getInnerElementType()");
    return getFirstInnerElement().getType();
}

TypedObject ArrayObject::getFirstInnerElement() const 
{
    assert (size() != 0 && "Zero-size array/vector getFirstInnerElement()");
    TypedObject element = (*this)[0];
    
    while (element.isArrayObject()) {
        element = element.getAs<ArrayObject>()->operator[](0);
    }

    return element;
}


std::vector<size_t> ArrayObject::dimensions() const {
    std::vector<size_t> resDim;
    resDim.push_back(size());

    if (size() > 0 && ArrayObject::getArrayObjectKind(getElementType())) {
        auto innerDims = (*this)[0].getAs<ArrayObject>()->dimensions();
        for (auto dim : innerDims)
            resDim.push_back(dim);
    }

    return resDim;
}

clang::QualType ArrayObject::getType() const
{
    return typedObj.getType();
}

const void *ArrayObject::getPtr() const
{
    return typedObj.getPtr();
}

bool ArrayObject::operator==(const ArrayObject &other) const
{
    return (kind == other.kind) && (typedObj == other.typedObj);
}

ArrayObject::ArrayObjectIterator ArrayObject::begin() const
{
    return ArrayObjectIterator(*this,0);
}

ArrayObject::ArrayObjectIterator ArrayObject::end() const
{
    return ArrayObjectIterator(*this,size());
}

ArrayObject::ArrayObjectIterator::ArrayObjectIterator(const ArrayObject &ao, size_t idx)
: ao(ao), idx(idx)
{}

ArrayObject::ArrayObjectIterator&
ArrayObject::ArrayObjectIterator::operator++()
{
    ++idx;
    return *this;
}

ArrayObject::ArrayObjectIterator
ArrayObject::ArrayObjectIterator::operator++(int)
{
    auto oldIter = *this;
    this->operator++();
    return oldIter;
}

TypedObject
ArrayObject::ArrayObjectIterator::operator*() const
{
    return ao[idx];
}

bool ArrayObject::ArrayObjectIterator::operator==(
    const ArrayObject::ArrayObjectIterator &other) const {
    return (idx == other.idx) && (ao == other.ao);
}

bool ArrayObject::ArrayObjectIterator::operator!=(
    const ArrayObject::ArrayObjectIterator &other) const {
    return !(*this == other);
}


// ---

ScObject::ScObject(TypedObject typedObj)
: RecordObject(typedObj)
{
}

const std::string & ScObject::scName() const {
    auto nameField = this->findField("m_name");
    return *reinterpret_cast<const std::string *>(nameField->typedObj.getPtr());
}

bool ScObject::isPort() const
{
    return isScBasePort(typedObj.getType());
}

bool ScObject::isInPort() const
{
    return isScIn(typedObj.getType());
}

bool ScObject::isOutPort() const
{
    return isScOut(typedObj.getType());
}

bool ScObject::isInOutPort() const
{
    return isScInOut(typedObj.getType());
}

bool ScObject::isSignal() const
{
    return isScSignal(typedObj.getType());
}

bool ScObject::isModule() const
{
    return isScModuleOrInterface(typedObj.getType());
}

bool ScObject::isModularInterface() const
{
    return sc::isScModularInterface(typedObj.getType());
}


} // namespace sc_elab
