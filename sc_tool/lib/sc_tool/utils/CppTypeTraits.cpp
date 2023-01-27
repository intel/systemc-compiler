/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "StringFormat.h"
#include "CppTypeTraits.h"
#include "ScTypeTraits.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include <iostream>

namespace sc {
    
using namespace clang;
using namespace std;

// Not used for now
char getRadix(const std::string& s) 
{
    auto i = s.begin();
    if (i == s.end()) return 10;
    
    if (*i == '-') {
        i++;
        if (i == s.end()) return 10;
    }
    
    if (*i == '0') {
        i++;
        if (i == s.end()) return 10;
        
        if (*i == 'x' || *i == 'X') {
            return 16;
        } else 
        if (*i == 'b' || *i == 'B') {
            return 2;
        } else 
        if (isdigit(*i)) {
            return 8;
        }
    }
    return 10;
}

// Get canonical type with qualifiers removed
clang::QualType getPureType(clang::QualType type) 
{
    type = type.getNonReferenceType();  // Remove reference
    type = type.getCanonicalType();
    type = type.getUnqualifiedType();
    return type;
}

// Get de-referenced type if this type is reference or this type otherwise
clang::QualType getDerefType(clang::QualType type) 
{
    if (!type.isNull() && type->isReferenceType()) {
        return type.getNonReferenceType();
    }
    return type;
}

// Is constant or constant reference type
bool isConstOrConstRef(QualType type) 
{
    return (!type.isNull() && 
            (type.getTypePtr()->isReferenceType() ?
             type.getNonReferenceType().isConstQualified() : 
             type.isConstQualified()));
}

// Check if the type is reference type
bool isReference(QualType type) 
{
    return (!type.isNull() && 
            type.getTypePtr()->isReferenceType());
}

// Check if the type is constant reference type
bool isConstReference(QualType type) 
{
    return (!type.isNull() && 
            type.getTypePtr()->isReferenceType() &&
            type.getNonReferenceType().isConstQualified());
}

// Check if the type is pure pointer 
bool isPointer(QualType type) 
{
    return (!type.isNull() && type.getTypePtr()->isPointerType());
}

// Constant type pointer: const T*
bool isConstPointer(QualType type) 
{
    return (!type.isNull() && 
            type.getTypePtr()->isPointerType() &&
            type.getTypePtr()->getPointeeType().isConstQualified());
}

// Check array of any type including std::array and std::vector, 
// \return number of array dimension
bool isArray(QualType type) 
{
    return (!type.isNull() && 
            (type->isArrayType() || sc::isStdArray(type) || sc::isStdVector(type)));
}

// Get array size from variable declaration or 0 if it is not array
size_t getArraySize(clang::ValueDecl* decl) 
{
    using namespace clang;
    QualType type = decl->getType();
    return getArraySize(type);
}

// Get array, std::array size from type 
// \return array size in elements or 0, 2 for int[2][3] 
size_t getArraySize(clang::QualType type) 
{
    using namespace clang;
    if (type.isNull()) return 0;
    
    if (type->isConstantArrayType()) {
        auto ctype = static_cast<const ConstantArrayType*>(type.getTypePtr());
        return ctype->getSize().getZExtValue();
    } else 
    if (isStdArray(type)) {
        auto sizeArg = sc::getTemplateArg(type, 1);
        size_t size = sizeArg->getAsIntegral().getZExtValue();
        return size;
    }
    // Cannot extract size for std::vector
    return 0;
}

// Get array, std::array, std::vector, sc_vector bottom element type
// \return int for int[2][3], but not int[3]
clang::QualType getArrayElementType(clang::QualType type)
{
    if (type.isNull()) return type;

    while (type->isArrayType()) {
        type = llvm::dyn_cast<clang::ArrayType>(type)->getElementType();
    }
    while (isStdArray(type) || isStdVector(type) || isScVector(type)) {
        auto elmType = getTemplateArgAsType(type, 0);
        type = *elmType;
    }
    return type;
}

// Get array, std::array, std::vector direct element type
// \return int[3] for int[2][3]
clang::QualType getArrayDirectElementType(clang::QualType type)
{
    if (type.isNull()) return type;

    if (type->isArrayType()) {
        type = llvm::dyn_cast<clang::ArrayType>(type)->getElementType();
    }
    if (isStdArray(type) || isStdVector(type)) {
        auto elmType = getTemplateArgAsType(type, 0);
        type = *elmType;
    }
    return type;
}

// Get array/sub-array sizes in multi-dimensional array
std::vector<std::size_t> getArraySizes(clang::QualType type)
{
    std::vector<std::size_t> arrSizes;
    if (type.isNull()) return arrSizes;

    // Fill @arrInds with array sizes
    while (type->isArrayType()) {
        arrSizes.push_back(getArraySize(type));
        type = clang::QualType(type->getArrayElementTypeNoTypeQual(), 0);
    }
    while (isStdArray(type)) {
        auto sizeArg = sc::getTemplateArg(type, 1);
        size_t size = sizeArg->getAsIntegral().getZExtValue();
        arrSizes.push_back(size);
        auto elmType = getTemplateArgAsType(type, 0);
        type = *elmType;
    }
    return arrSizes;
}

// Get total element number in one/multi-dimensional array, 
// for one-dimensional array the same as its size
std::size_t getArrayElementNumber(clang::QualType type)
{
    if (type.isNull()) return 0;

    auto arrSizes = getArraySizes(type);
    return getArrayElementNumber(arrSizes);
}

// Get total element number in one/multi-dimensional array, 
// for one-dimensional array the same as its size
std::size_t getArrayElementNumber(const vector<size_t>& arrSizes)
{
    std::size_t elmnum = (arrSizes.size() > 0) ? 1 : 0;

    for (auto s : arrSizes) {
        elmnum = elmnum*s;
    }

    return elmnum;
}


// Get array indices in multi-dimensional for given @indx
std::vector<std::size_t> getArrayIndices(clang::QualType type, std::size_t indx) 
{
    std::vector<std::size_t> arrInds = getArraySizes(type);
    return getArrayIndices(arrInds, indx);
}

// Get array indices in multi-dimensional for given @indx
// \param allSizes -- record array and field array joined sizes
std::vector<std::size_t> getArrayIndices(const vector<size_t>& arrSizes, 
                                         std::size_t indx) 
{
    std::vector<std::size_t> arrInds(arrSizes);

    // Fill @arrInds with element indices
    for (auto i = arrInds.rbegin(); i != arrInds.rend(); ++i) {
        std::size_t a = indx % (*i);
        indx = indx / (*i);
        *i = a;
    }
    return arrInds;
}


// Check if the type is pointer to constant type
bool isPointerToConst(clang::QualType type)
{
    if (type.isNull()) return false;
    
    return (!type.isNull() && 
            type.getTypePtr()->isPointerType() && 
            type.getTypePtr()->getPointeeType().isConstQualified());
}

bool isBoolType(clang::QualType type)
{
    // Get the most inner array element type
    if (type.isNull()) return false;
    
    QualType ctype = type->getPointeeOrArrayElementType()->
                     getCanonicalTypeInternal();
    if (ctype.isNull()) return false;
    
    if (auto btype = dyn_cast<BuiltinType>(ctype.getTypePtr())) {
        auto kind = btype->getKind();
        return (kind == BuiltinType::Kind::Bool);
    }
    return false;
}

bool isVoidType(clang::QualType type) 
{
    // Get the most inner array element type
    if (type.isNull()) return false;
    
    QualType ctype = type->getCanonicalTypeInternal();
    if (ctype.isNull()) return false;
    
    if (auto btype = dyn_cast<BuiltinType>(ctype.getTypePtr())) {
        auto kind = btype->getKind();
        return (kind == BuiltinType::Kind::Void);
    }
    return false;
}

bool isCharType(clang::QualType type) 
{
    // Get the most inner array element type
    if (type.isNull()) return false;
    
    QualType ctype = type->getCanonicalTypeInternal();
    if (ctype.isNull()) return false;
    
    if (auto btype = dyn_cast<BuiltinType>(ctype.getTypePtr())) {
        auto kind = btype->getKind();
        return (kind == BuiltinType::Kind::UChar || 
                kind == BuiltinType::Kind::Char_U ||
                kind == BuiltinType::Kind::Char8 ||
                kind == BuiltinType::Kind::SChar ||
                kind == BuiltinType::Kind::Char_S);
    }
    return false;
}

// Check if the type is std::string
bool isStdString(clang::QualType type)
{
    if (type.isNull()) return false;
    
    std::string typeName = type->getCanonicalTypeInternal().getAsString();
    return (typeName.find("std::") != std::string::npos && 
            typeName.find("basic_string<char>") != std::string::npos);
}

// Check if the type is const char *
bool isConstCharPtr(QualType type)
{
    if (type.isNull()) return false;
    
    if (!isPointer(type)) return false;
    QualType ctype = type->getPointeeType();

    if (!ctype.isConstQualified()) return false;
    ctype = ctype->getCanonicalTypeInternal();
    
    return isCharType(ctype);
}

// Check if the type is cin/cout
bool isIoStream(QualType type) 
{
    if (type.isNull()) return false;
    
    string typeStr = type.getAsString();
    return (typeStr.find("basic_ostream") != string::npos || 
            typeStr.find("basic_istream") != string::npos || 
            typeStr.find("std::ostream") != string::npos || 
            typeStr.find("std::istream") != string::npos);
}

// Check SC module or CXX class/structure, but not SC channel or SC data type
// Do not check reference, use type.getNonReferenceType() if required
bool isUserClass(clang::QualType type, bool checkPointer) 
{
    if (type.isNull()) return false;

    QualType ctype = type;
    if (checkPointer) {
        while (ctype->isPointerType()) {
            ctype = ctype->getPointeeType();
        }
    }
    
    // Record types, union type is not supported
    if (!ctype->isStructureOrClassType()) {
        return false;
    }
    if (isScChannel(ctype) || isScVector(ctype)) {
        return false;
    }
    if (isAnyScIntegerRef(ctype)) {
        return false;
    }
    if (isStdArray(ctype) || isStdVector(ctype)) {
        return false;
    }
    return true;
}

// Check array/vector of any class/structure/module type
// \param checkPointer -- check array of pointers to class
bool isUserDefinedClassArray(QualType type, bool checkPointer) 
{
    if (type.isNull()) return false;

    type = getArrayElementType(type);

    if (checkPointer) {
        if (isPointer(type)) {
            type = type->getPointeeType();
        }
        if (isScPort(type)) {
            return true;
        }
    }

    return (isUserClass(type));
}

// Get user defined class from array/vector or none
llvm::Optional<QualType> getUserDefinedClassFromArray(QualType type) 
{
    if (type.isNull()) return llvm::None;

    type = getArrayElementType(type);

    if (isUserClass(type)) { 
        return type;
    } else {
        return llvm::None;
    }
}

// Check if a class declaration is template
bool isTemplateClass(clang::CXXRecordDecl* decl) {
    return decl->getDescribedClassTemplate();
}

unsigned getTemplateArgNum(clang::QualType type)
{
    if (type.isNull()) return 0;
    
    type = getPureType(type);
    
    if (auto stype = type->getAs<clang::TemplateSpecializationType>()) {
        return stype->getNumArgs();
    } else
    if (auto rdecl = type->getAsCXXRecordDecl()) {
        if (auto sdecl = dyn_cast<clang::ClassTemplateSpecializationDecl>(rdecl)) {
            return sdecl->getTemplateArgs().size();
        }
     }
    return 0;
}

llvm::Optional<TemplateArgument> getTemplateArg(clang::QualType type, 
                                                std::size_t argIndx)
{
    if (type.isNull()) return llvm::None;
    
    type = getPureType(type);

    if (auto stype = type->getAs<clang::TemplateSpecializationType>()) {
        if (stype->getNumArgs() > argIndx) {
            return stype->getArg(argIndx);
        }
     } else 
     if (auto rdecl = type->getAsCXXRecordDecl()) {
        if (auto sdecl = dyn_cast<clang::ClassTemplateSpecializationDecl>(rdecl)) {
            if (sdecl->getTemplateArgs().size() > argIndx) {
                return sdecl->getTemplateArgs().operator [](argIndx);
            }
        }
    }
    return llvm::Optional<TemplateArgument>();
}

llvm::Optional<clang::QualType> getTemplateArgAsType(clang::QualType type, 
                                                     std::size_t argIndx)
{
    if (type.isNull()) return llvm::None;
    
    auto tmplArg = getTemplateArg(type, argIndx);
    if (tmplArg && tmplArg->getKind() == TemplateArgument::ArgKind::Type) {
        return tmplArg->getAsType();
    }
   
    return llvm::Optional<clang::QualType>();
}

llvm::Optional<llvm::APSInt> getTemplateArgAsInt(clang::QualType type, 
                                                 std::size_t argIndx)
{
    if (type.isNull()) return llvm::None;
    
    auto tmplArg = getTemplateArg(type, argIndx);
    if (tmplArg && tmplArg->getKind() == TemplateArgument::ArgKind::Integral) {
        return tmplArg->getAsIntegral();
    }
    
    return llvm::Optional<llvm::APSInt>();
}

// Cast integer to the given type width and sign
llvm::APSInt extrOrTrunc(const llvm::APSInt& val, size_t resWidth, bool isUnsigned)
{
    if (val.isUnsigned()) {
        return llvm::APSInt(val.zextOrTrunc(resWidth), isUnsigned);
    } else {
        return llvm::APSInt(val.sextOrTrunc(resWidth), isUnsigned);
    }
}

// Extend values width to its type width before operation to fit result value
void extendBitWidthBO(llvm::APSInt& val1, llvm::APSInt& val2, 
                      size_t width1, size_t width2)
{
    SCT_TOOL_ASSERT (width1 > 0 && width2 > 0, 
                     "Incorrect width in extendBitWidthBO");
    
    if (val1.getBitWidth() < width1) {
        val1 = val1.extend(width1);
    }
    if (val2.getBitWidth() < width2) {
        val2 = val2.extend(width2);
    }
}

// Extend values width to result width before operation to fit result value
void extendBitWidthOO(llvm::APSInt& val1, llvm::APSInt& val2, 
                      size_t width1, size_t width2,
                      const OverloadedOperatorKind& opcode)
{
    using namespace llvm;
    // Use given data type width if determined
    width1 = width1 ? width1 : getBitsNeeded(val1);
    width2 = width2 ? width2 : getBitsNeeded(val2);
    unsigned maxwidth = (width1 > width2) ? width1 : width2;
    
    unsigned width = 64;
    if (opcode == OO_Plus || opcode == OO_Minus || opcode == OO_PlusEqual || 
        opcode == OO_MinusEqual) {
        width = maxwidth + 1;
        
    } else 
    if (opcode == OO_Star || opcode == OO_StarEqual) {
        width = width1 + width2;
        
    } else 
    if (opcode == OO_Slash || opcode == OO_Percent || opcode == OO_SlashEqual || 
        opcode == OO_PercentEqual) {
        width = width1;
        
    } else 
    if (opcode == OO_GreaterGreater || opcode == OO_GreaterGreaterEqual) {
        if (val2.isNegative()) {
            ScDiag::reportScDiag(ScDiag::SYNTH_NEGATIVE_SHIFT);
        }
        width = width1;
        
    } else 
    if (opcode == OO_LessLess || opcode == OO_LessLessEqual) {
        if (val2.isNegative()) {
            ScDiag::reportScDiag(ScDiag::SYNTH_NEGATIVE_SHIFT);
        }
        unsigned shift = val2.getExtValue();
        if (shift > 1024) {
            ScDiag::reportScDiag(ScDiag::SYNTH_BIG_SHIFT);
        }
        width = width1 + shift;
        
    } else
    if (opcode == OO_EqualEqual || opcode == OO_ExclaimEqual || 
        opcode == OO_Less || opcode == OO_LessEqual || 
        opcode == OO_Greater || opcode == OO_GreaterEqual) {
        width = 1;
        
    } else
    if (opcode == OO_Caret || opcode == OO_Amp || opcode == OO_Pipe || 
        opcode == OO_AmpEqual || opcode == OO_PipeEqual || 
        opcode == OO_CaretEqual) {
        width = maxwidth;
    } 
    //cout << "extendBitWidthOO opcode " << opcode << " width " << width << endl;
            
    if (val1.getBitWidth() < width) {
        val1 = val1.extend(width);
    }
    if (val2.getBitWidth() < width) {
        val2 = val2.extend(width);
    }
}

// Adjust APSInt to the same sign and maximal bit width
void adjustIntegers(llvm::APSInt val1, llvm::APSInt val2, llvm::APSInt &res1,
                    llvm::APSInt &res2, bool promoteSigned)
{
    res1 = val1;
    res2 = val2;

    if (promoteSigned) {
        // Type promotion to signed -- corresponds to SC datatype semantic
        if (res1.isUnsigned() != res2.isUnsigned()) {
            if (res1.isUnsigned()) {
                res1 = res1.extend(res1.getBitWidth() + 1);
                res1.setIsUnsigned(false);
            } else 
            if (res2.isUnsigned()) {
                res2 = res2.extend(res2.getBitWidth() + 1);
                res2.setIsUnsigned(false);
            }
        }
        
        if (res1.getBitWidth() > res2.getBitWidth()) {
            res2 = res2.extend(res1.getBitWidth());
        } else 
        if (res2.getBitWidth() > res1.getBitWidth()) {
            res1 = res1.extend(res2.getBitWidth());
        }
        
    } else {
        // Type promotion to unsigned -- corresponds to CPP and Verilog semantic
        if (res1.getBitWidth() > res2.getBitWidth()) {
            res2 = res2.extend(res1.getBitWidth());
        } else 
        if (res2.getBitWidth() > res1.getBitWidth()) {
            res1 = res1.extend(res2.getBitWidth());
        }

        if (res1.isUnsigned() || res2.isUnsigned()) {
            res1.setIsUnsigned(true);
            res2.setIsUnsigned(true);
        }
    }
}

unsigned getBitsNeeded(llvm::APSInt val) 
{
    if (val.isNullValue()) {
        return 1;
    } else {
        if (val < 0) {
            val = val * (-1);
            return (val.getActiveBits() + 1);
        } else {
            return val.getActiveBits();
        }
    }
}

// Try to get get @CXXConstructExpr from given expression
CXXConstructExpr* getCXXCtorExprArg(Expr* expr)
{
    if (auto bindtmp = dyn_cast<CXXBindTemporaryExpr>(expr)) {
        expr = bindtmp->getSubExpr();
    }
    return dyn_cast<CXXConstructExpr>(expr);
}

llvm::Optional<std::string> getNamespaceAsStr(const clang::Decl *decl)
{
    const auto *declCtx = decl->getDeclContext();

    if (!declCtx->isNamespace()) {
        declCtx = declCtx->getEnclosingNamespaceContext();
    }

    if (!declCtx->isNamespace())
        return llvm::None;

    auto nd = llvm::cast<clang::NamespaceDecl>(declCtx);
    return std::string(nd->getName());
}

bool isLinkageDecl(const clang::Decl* decl)
{
    return decl->getDeclContext()->getDeclKind() == clang::Decl::Kind::LinkageSpec;
}

clang::Expr* removeExprCleanups(clang::Expr* expr) 
{
    while (auto cuExpr = dyn_cast<ExprWithCleanups>(expr)) {
        expr = cuExpr->getSubExpr();
    }
    return expr;
}

clang::Stmt* removeExprCleanups(clang::Stmt* stmt) 
{
    while (auto cuExpr = dyn_cast<ExprWithCleanups>(stmt)) {
        stmt = cuExpr->getSubExpr();
    }
    return stmt;
}

// Check if expression contains a sub-expression of boolean type
bool isBoolArgument(const Expr* expr) 
{
    using namespace clang;
    if (expr && isBoolType(expr->getType())) return true;

    while (auto castExpr = dyn_cast<const CastExpr>(expr)) {
        expr = castExpr->getSubExpr();
        if (!expr) break;
        
        if (isBoolType(expr->getType())) return true;
    }
    
    return false;
}

}