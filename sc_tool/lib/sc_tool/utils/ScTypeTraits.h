/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 * Modified by: Mikhail Moiseev
 */

#ifndef SCTOOL_SC_TYPE_TRAITS_H
#define SCTOOL_SC_TYPE_TRAITS_H

#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/AST/ASTContext.h>
#include <llvm/ADT/SmallSet.h>

// Functions to query for various properties of types defined in ASTContext

namespace sc {
    
struct IntTraits {
    IntTraits (size_t width, bool isSigned)
    : width(width), isSigned(isSigned) {}

    size_t width;
    bool isSigned;
};

/// Initializes database of built-in types, should be called before any
/// type traits query
void initTypeTraits(const clang::ASTContext& astCtx);

/// Returns true if funcDecl is inside std, sc_core or sc_dt namespaces
bool isStdFuncDecl(const clang::FunctionDecl *funcDecl);

/// Check if funcDecl is SystemC wait()
bool isWaitDecl(const clang::FunctionDecl *funcDecl);

/// Check is type is any signed type
bool isSignedType(clang::QualType type);

/// Check if type is SystemC integer
bool isScInt(clang::QualType type);
bool isScUInt(clang::QualType type);
bool isScBigInt(clang::QualType type);
bool isScBigUInt(clang::QualType type);
/// Check for @sc_bv
bool isScBitVector(clang::QualType type);

/// Check for @sct_zero_width or sc_signal/sc_in/sc_out of @sct_zero_width type
/// \return false for SS channel of @sct_zero_width
bool isZeroWidthType(clang::QualType type);
bool isZeroWidthArrayType(clang::QualType type);

/// Is any of SystemC integer type
bool isAnyScInteger(clang::QualType type);
/// Is any of SystemC integer type or SystemC subref/concatref
bool isAnyScIntegerRef(clang::QualType type, bool checkPointer = false);
/// Is not supported SC integer types
bool isScNotSupported(clang::QualType type, bool checkPointer);

/// Is any of C++ built-in, enum, SystemC integer type or SystemC subref/concatref
bool isAnyIntegerRef(clang::QualType type);
/// Is any of C++ built-in, enum, SystemC integer type 
bool isAnyInteger(clang::QualType type);

/// Check array of @sc_int/@sc_bigint/@sc_uint/@sc_biguint type or 
/// pointer to such array
/// \param checkPointer -- check if it is pointer to array
bool isScIntegerArray(clang::QualType type, bool checkPointer = true);

/// Check @sc_uint or @sc_biguint or @sc_bv type and optionally return bit number
llvm::Optional<size_t> getScUintBiguintBitVec(clang::QualType type);
/// Check @sc_int or @sc_bigint type and optionally return bit number
llvm::Optional<size_t> getScIntBigint(clang::QualType type);

/// Check type is @sc_signed which is base class for @sc_bigint but not template, 
/// used for operation result of @sc_bigint
bool isScSigned(clang::QualType type);
/// Check type is @sc_unsigned which is base class for @sc_biguint but not template, 
/// used for operation result of @sc_biguint
bool isScUnsigned(clang::QualType type);

/// Try to get integer type for which width can be obtained by @getIntTraits()
clang::QualType getTypeForWidth(const clang::Expr* expr);

/// Get width of any integral type including SC data types
/// \return < width, isUnsigned >
llvm::Optional<std::pair<size_t, bool> > getIntTraits(clang::QualType type, 
                                                      bool checkPointer = false);

/// Get width of any integer type wrapped into given @type, based on @getIntTraits
/// Can work for any integral type, integral pointer, channel, pointer to channel
llvm::Optional<size_t> getAnyTypeWidth(clang::QualType type, bool checkPointer, 
                                       bool checkChannel);

/// Check for module only
bool isScModule(clang::QualType type, bool checkPointer = false);
/// Check for module or modular interface
bool isScModuleOrInterface(clang::QualType type, bool checkPointer = false);
/// Check for modular interface only
bool isScModularInterface(clang::QualType type);

bool isScObject(clang::QualType type);

bool isScVector(clang::QualType type);
bool isStdVector(clang::QualType type);
bool isStdArray(clang::QualType type);

/// @sc_base_port
bool isScBasePort(clang::QualType type);
/// @sc_port<IF>
bool isScPort(clang::QualType type);
bool isScIn(clang::QualType type);
bool isScOut(clang::QualType type);
bool isScInOut(clang::QualType type);

/// Check if the type is pointer or @sc_port<IF>
bool isPointerOrScPort(clang::QualType type);

/// Check if @type is sc_signal or its inheritor
bool isScSignal(clang::QualType type);

/// Check if @type is sct_comb_signal or its inheritor
bool isScToolCombSignal(clang::QualType type, bool checkPointer = true);
/// Check if @type is sct_clear_signal or its inheritor
bool isScToolClearSignal(clang::QualType type, bool checkPointer = true);

/// Check if type is signal or any kind of port
/// \param checkPointer -- also check if it is pointer to signal
bool isScChannel(clang::QualType type, bool checkPointer = true);

/// Check array, sc_vector, std::array(?), std::vector(?) of any SC channel type
/// \param checkPointer -- check array of pointers to channel
bool isScChannelArray(clang::QualType type, bool checkPointer = true);

/// Get record type if it is SC channel of record type, or none
llvm::Optional<clang::QualType>  
isUserClassChannel(clang::QualType type, bool checkPointer = true);

/// Any the type in sc_core namespace
bool isAnyScCoreObject(clang::QualType type);

/// \return true if type is in sc_core namespace
bool isScCoreType(clang::QualType type);

/// ...
bool isSctFifo(clang::QualType type);
bool isSctTarg(clang::QualType type);
bool isSctCombTarg(clang::QualType type);

/// Check for SS channels used in sensitivity lists
/// For ff_synchronizer only read()/operator bool() required to add in sensitivity 
bool isSctChannelSens(clang::QualType type, const clang::FunctionDecl* funcDecl);

/// Check if operator = is supported for type, sct_register and sct_ff_synchronizer
bool isAssignOperatorSupported(clang::QualType type);

bool isScProcess(clang::QualType type);
bool isScMethod(clang::QualType type);
bool isScThread(clang::QualType type);
bool isScCThread(clang::QualType type);

bool isSignedOrArrayOfSigned(clang::QualType type);

clang::QualType getScPortBaseType();
clang::QualType getScProcessBType();
clang::QualType getScSimContextType();

} // namespace sc


#endif //SCTOOL_SC_TYPE_TRAITS_H
