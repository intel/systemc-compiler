/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * CPP types checking functions
 * Author: Roman Popov
 */

#ifndef CPPTYPETRAITS_H
#define CPPTYPETRAITS_H

#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"
#include "clang/AST/ExprCXX.h"
#include "llvm/ADT/APSInt.h"

namespace sc
{
/// Evaluation precision 
const uint32_t EPRECISION = 64;

/// Get radix from number in the string, string must contain correct number
char getRadix(const std::string& s); 

// Get canonical type with qualifiers removed, 
// must be used to identify "const T" as T with @DeclDB
clang::QualType getPureType(clang::QualType type);

/// Get de-referenced type if this type is reference or this type otherwise
clang::QualType getDerefType(clang::QualType type);

/// Is constant or constant reference type
bool isConstOrConstRef(clang::QualType type);

/// Check if the type is reference type
bool isReference(clang::QualType type);

/// Check if the type is constant reference type
bool isConstReference(clang::QualType type);

/// Check if the type is pure pointer 
bool isPointer(clang::QualType type);

/// Constant type pointer: const T*
bool isConstPointer(clang::QualType type);

/// Check array of any type including std::array and std::vector, 
/// \return number of array dimension
bool isArray(clang::QualType type);

/// Get array size from variable declaration or 0 if it is not array
size_t getArraySize(clang::ValueDecl* decl);

/// Get array, std::array size from type 
/// \return array size in elements or 0, 2 for int[2][3]  
size_t getArraySize(clang::QualType type);

/// Get array, std::array, std::vector, sc_vector element type
/// \return int for int[2][3], but not int[3]
clang::QualType getArrayElementType(clang::QualType type);

// Get array, std::array, std::vector direct element type
// \return int[3] for int[2][3]
clang::QualType getArrayDirectElementType(clang::QualType type);

/// Get array/sub-array sizes in multi-dimensional array
std::vector<std::size_t> getArraySizes(clang::QualType type);

/// Get total element number in one/multi-dimensional array, 
/// for one-dimensional array the same as its size
std::size_t getArrayElementNumber(clang::QualType type);
std::size_t getArrayElementNumber(const std::vector<size_t>& allSizes);

/// Get array indices in multi-dimensional for given @indx
std::vector<std::size_t> getArrayIndices(clang::QualType type, std::size_t indx);
std::vector<std::size_t> getArrayIndices(const std::vector<size_t>& allSizes, 
                                         std::size_t indx);

/// Check if the type is pointer to constant type
bool isPointerToConst(clang::QualType type);

/// Check if the type is bool
bool isBoolType(clang::QualType type);

/// Check if the type is void
bool isVoidType(clang::QualType type);

/// Check if the type is any kind of char
bool isCharType(clang::QualType type); 

/// Check if the type is std::string
bool isStdString(clang::QualType type);

/// Check if the type is const char *
bool isConstCharPtr(clang::QualType type);

/// Check if the type is IO stream
bool isIoStream(clang::QualType type);

/// Check SC module or CXX class/structure, but not SC channel or SC data type
/// Do not check reference, use type.getNonReferenceType() if required
bool isUserClass(clang::QualType type, bool checkPointer = false);

/// Check array/vector of any class/structure/module type
/// \param checkPointer -- check array of pointers to class
bool isUserDefinedClassArray(clang::QualType type, bool checkPointer);

/// Get user defined class from array/vector or none
llvm::Optional<clang::QualType> getUserDefinedClassFromArray(clang::QualType type);

/// Check if a class declaration is template
bool isTemplateClass(clang::CXXRecordDecl *decl);

/// Get template argument number or zero
unsigned getTemplateArgNum(clang::QualType type);

/// Get template argument at index argIndx or nothing
llvm::Optional<clang::TemplateArgument> 
getTemplateArg(clang::QualType type, std::size_t argIndx);

/// Get template argument at index argIndx as type,
/// return none if it is nota type or there less arguments
llvm::Optional<clang::QualType> 
getTemplateArgAsType(clang::QualType type, std::size_t argIndx);

/// Get template argument at index argIndx as integral value, 
/// return none if it is nota type or there less arguments
llvm::Optional<llvm::APSInt> 
getTemplateArgAsInt(clang::QualType type, std::size_t argIndx);

/// Cast integer to the given type width and sign
llvm::APSInt extrOrTrunc(const llvm::APSInt &val, size_t resWidth, bool isUnsigned);

/// Extend values width to its type width before operation to fit result value
/// \param width1, width2 -- type width
void extendBitWidthBO(llvm::APSInt& val1, llvm::APSInt& val2, 
                      size_t width1, size_t width2);

/// Extend values width to result width before operation to fit result value
/// \param width1, width2 -- type width, may be zero for sc_(un)signed
/// \param opcode -- operation used to evaluate result width
void extendBitWidthOO(llvm::APSInt& val1, llvm::APSInt& val2, 
                      size_t width1, size_t width2,
                      const clang::OverloadedOperatorKind& opcode);

/// Adjust APSInt to the same sign and maximal bit width
void adjustIntegers(llvm::APSInt val1, llvm::APSInt val2, llvm::APSInt &res1,
                    llvm::APSInt &res2, bool promoteSigned = false);

/// Get number of bits required for signed/unsigned literal
unsigned getBitsNeeded(llvm::APSInt val);

/// Try to get get CXXConstructExpr from given expression
clang::CXXConstructExpr* getCXXCtorExprArg(clang::Expr* expr);

/// Get namespace name string from declaration or none
llvm::Optional<std::string> getNamespaceAsStr(const clang::Decl *decl);

/// Check if declaration context of @decl has LinkageSpec kind, 
/// used for @std functions 
bool isLinkageDecl(const clang::Decl* decl);

/// Remove @ExprWithCleanups from @expr 
clang::Expr* removeExprCleanups(clang::Expr* expr);
clang::Stmt* removeExprCleanups(clang::Stmt* stmt);

/// Check if expression contains a sub-expression of boolean type
bool isBoolArgument(const clang::Expr* expr);

}

#endif /* CPPTYPETRAITS_H */

