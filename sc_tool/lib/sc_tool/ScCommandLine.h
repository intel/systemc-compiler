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
extern llvm::cl::opt<bool>          noRemoveExtraCode;
extern llvm::cl::opt<bool>          initLocalVars;
extern llvm::cl::opt<bool>          keepConstVariables;

// Planned to remove
extern llvm::cl::opt<bool>          noProcessAnalysis;
extern llvm::cl::opt<bool>          constPropOnly;
extern llvm::cl::opt<bool>          singleBlockCThreads;

// Enable individual readability features
inline unsigned ENABLE_REMOVE_EXTRA_CODE;
enum codeRemoveOpt {crResetUnused = 1, crBodyUnused = 2, crUnusedNext = 4,
                    crEmptyFunc = 8, crEmptyIf = 16, crEmptyLoop = 32,
                    crRemoveConst = 64};

// Remove unusable variables in reset section of CTHREAD
inline bool REMOVE_RESET_UNUSED() {
    return (ENABLE_REMOVE_EXTRA_CODE & codeRemoveOpt::crResetUnused);
}

// Remove unusable variables after reset in CTHREAD
inline bool REMOVE_BODY_UNUSED() {
    return (ENABLE_REMOVE_EXTRA_CODE & codeRemoveOpt::crBodyUnused);
}

// Remove next value of register which not used after reset in CTHREAD and SVA
inline bool REMOVE_UNUSED_NEXT() {
    return (ENABLE_REMOVE_EXTRA_CODE & codeRemoveOpt::crUnusedNext);
}

// Remove empty function comments 
inline bool REMOVE_EMPTY_FUNC() {
    return (ENABLE_REMOVE_EXTRA_CODE & codeRemoveOpt::crEmptyFunc);
}

// Remove empty IF statement and IF then/else branches
inline bool REMOVE_EMPTY_IF() {
    return (ENABLE_REMOVE_EXTRA_CODE & codeRemoveOpt::crEmptyIf);
}

// Remove empty loop statement and body 
inline bool REMOVE_EMPTY_LOOP() {
    return (ENABLE_REMOVE_EXTRA_CODE & codeRemoveOpt::crEmptyLoop);
}

// Remove constant declaration 
inline bool REMOVE_CONST_DECL() {
    return (!keepConstVariables && 
            (ENABLE_REMOVE_EXTRA_CODE & codeRemoveOpt::crRemoveConst));
}

#endif //SCTOOL_SCCOMMANDLINE_H
