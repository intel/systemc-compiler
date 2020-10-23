/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "ScCfgCursor.h"

namespace sc {

const clang::CFGBlock *curBlock(const CfgCursorStack &stack)
{
    if (stack.empty())
        return nullptr;
    return stack.back().getBlock();
}

size_t curElem(const CfgCursorStack &stack)
{
    if (stack.empty())
        return 0;
    return stack.back().getElementID();
}

} // namespace sc

