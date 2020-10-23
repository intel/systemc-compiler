/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCCFGANALYSIS_H
#define SCTOOL_SCCFGANALYSIS_H

#include <clang/Analysis/CFG.h>
#include <clang/AST/Stmt.h>

// Helper functions for Clang CFG analysis

namespace sc {

/// If entryBlock is a block with conditional stmt terminator (if or switch),
/// returns first block after conditional
const clang::CFGBlock* getCondExitBlock(const clang::CFGBlock * entryBlock);

/// If possible, get block that follows loop or conditional
const clang::CFGBlock * getCondOrLoopExitBlock(
    const clang::CFGBlock *entryBlock, const clang::Stmt *headerStmt);

/// If block is loop header (first block of loop), returns back edge block
const clang::CFGBlock* getLoopBackEdge( const clang::CFGBlock *loopHeaderBlock);

/// If block is loop header (first block of loop), returns loop Stmt
const clang::Stmt* getLoopStmt(const clang::CFGBlock *loopHeaderBlock);

/// Check if current block is loop header (start of loop)
bool isLoopHeaderBlock (const clang::CFGBlock *cfgBlock);

bool isLoopStmt(const clang::Stmt* stmt);
bool isCondStmt(const clang::Stmt* stmt);

///// If block is a loop header or condtional header, returns corresponding Stmt
//const clang::Stmt* getLoopOrCondHeaderStmt(const clang::CFGBlock *headerBlock);

/// Return true if block is loop back edge block
bool isLoopBackEdge(const clang::CFGBlock *block);

}

#endif //SCTOOL_SCCFGANALYSIS_H
