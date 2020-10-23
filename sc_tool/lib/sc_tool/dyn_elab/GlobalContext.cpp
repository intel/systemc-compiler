/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/dyn_elab/GlobalContext.h>

namespace sc_elab
{

static clang::ASTContext * globalAstCtx = nullptr;
static MangledTypeDB *mangledTypeDB = nullptr;

void initGlobalContext(clang::ASTContext *astCtx, MangledTypeDB *typeDB)
{
    globalAstCtx = astCtx;
    mangledTypeDB = typeDB;
}

clang::ASTContext *getAstCtx()
{
    assert(globalAstCtx != nullptr);
    return globalAstCtx;
}

MangledTypeDB *getMangledTypeDB()
{
    assert(mangledTypeDB != nullptr);
    return mangledTypeDB;
}

}

