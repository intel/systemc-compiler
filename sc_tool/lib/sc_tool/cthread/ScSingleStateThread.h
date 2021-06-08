/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCSINGLESTATETHREAD_H
#define SCTOOL_SCSINGLESTATETHREAD_H

#include <clang/AST/Decl.h>
#include <sc_tool/cthread/ScCThreadStates.h>

namespace sc {

/**
 * Detect if a thread has a only a single state
 *
 * Currently it uses the following algorithm:
 * if the first statement after each wait() call is a loop,
 * and if this loop is the same for all wait()s, then return true.
 * Otherwise return false
 *
 * @param funcDecl - top level function for thread
 * @return <one wait or all waits before/in the end of main loop, 
 *          only one wait in the end of main loop>
 */
std::pair<bool, bool> isSingleStateThread(const ScCThreadStates &states, 
                                          const clang::Stmt* mainLoop);

} // namespace sc

#endif //SCTOOL_SCSINGLESTATETHREAD_H
