/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_GLOBALCONTEXT_H
#define SCTOOL_GLOBALCONTEXT_H

#include <sc_tool/dyn_elab/MangledTypeDB.h>
#include <clang/AST/ASTContext.h>

namespace sc_elab {

clang::ASTContext * getAstCtx();
MangledTypeDB * getMangledTypeDB();

void initGlobalContext(clang::ASTContext *astCtx, MangledTypeDB *typeDB);

}

#endif //SCTOOL_GLOBALCONTEXT_H
