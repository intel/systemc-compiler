/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_MANGLEDTYPES_H
#define SCTOOL_MANGLEDTYPES_H

#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>

#include <unordered_map>

namespace sc_elab {

/// Maps mangled type name to clang::QualType
struct MangledTypeDB
{
    MangledTypeDB(clang::ASTContext &astCtx);

    clang::QualType getType(llvm::StringRef mangledTypeName);

private:

    std::unordered_map<std::string, clang::QualType> typeMap;

};

} // namespace sc_elab


#endif //SCTOOL_MANGLEDTYPES_H
