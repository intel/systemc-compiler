/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCCOMMANDLINE_H
#define SCTOOL_SCCOMMANDLINE_H

#include <llvm/Support/CommandLine.h>
#include <string>

extern llvm::cl::OptionCategory ScToolCategory;

extern llvm::cl::opt<std::string>   verilogFileName;
extern llvm::cl::opt<bool>          noSvaGenerate;
extern llvm::cl::opt<bool>          portMapGenerate;
extern llvm::cl::opt<bool>          noRemoveExtraCode;
extern llvm::cl::opt<bool>          checkUnsigned;
extern llvm::cl::opt<bool>          initLocalVars;
extern llvm::cl::opt<bool>          initResetLocalVars;
extern llvm::cl::opt<std::string>   modulePrefix;

// Remove unusable variables in reset section of CTHREAD
inline bool REMOVE_RESET_UNUSED() {
    return !noRemoveExtraCode;
}

// Remove unusable variables after reset in CTHREAD
inline bool REMOVE_BODY_UNUSED() {
    return !noRemoveExtraCode;
}

// Remove next value of register which not used after reset in CTHREAD and SVA
inline bool REMOVE_UNUSED_NEXT() {
    return !noRemoveExtraCode;
}

// Remove empty function comments 
inline bool REMOVE_EMPTY_FUNC() {
    return !noRemoveExtraCode;
}

// Remove empty IF statement and IF then/else branches
inline bool REMOVE_EMPTY_IF() {
    return !noRemoveExtraCode;
}

// Remove empty loop statement and body 
inline bool REMOVE_EMPTY_LOOP() {
    return !noRemoveExtraCode;
}

// Remove unused variable definition statements in METHODs and CTHREADs, 
// that works in @ScTraverseConst::removeUnusedStmt()
inline bool REMOVE_UNUSED_VAR_STMTS() {
    return false;  // #196
    //return !noRemoveExtraCode;
}

#endif //SCTOOL_SCCOMMANDLINE_H
