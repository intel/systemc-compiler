/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "sc_tool/utils/DebugOptions.h"
#include <llvm/Support/Debug.h>

using namespace sc;

bool enabledFlag = false;
    
bool DebugOptions::isEnabled(const char *optName)
{
#ifndef NDEBUG
    return ::llvm::DebugFlag && ::llvm::isCurrentDebugType(optName);
#else
    return false;
#endif
}

void DebugOptions::enable(const char **optNames, unsigned count) 
{
#ifndef NDEBUG
    ::llvm::setCurrentDebugTypes(optNames, count);
    ::llvm::DebugFlag = true;
    enabledFlag = true;
#endif
}

bool DebugOptions::isDebug() {
#ifndef NDEBUG
    return ::llvm::DebugFlag;
#else 
    return false;
#endif
}

void DebugOptions::suspend() {
#ifndef NDEBUG
    ::llvm::DebugFlag = false;
#endif
}

void DebugOptions::resume() {
#ifndef NDEBUG
    ::llvm::DebugFlag = enabledFlag;
#endif
}

