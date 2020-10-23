/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCTHREADSTMTVISITORS_H
#define SCTOOL_SCTHREADSTMTVISITORS_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <sc_tool/utils/ScTypeTraits.h>
#include <unordered_map>

namespace sc {

class FindWaitCallVisitor
    : public clang::RecursiveASTVisitor<FindWaitCallVisitor> {

    mutable bool foundStmt = false;
    mutable bool searchWait = false;
    mutable bool searchBreak = false;
    mutable bool searchContinue = false;

    mutable std::unordered_map<const clang::Stmt*, bool> hasWaitMap;

public:
    bool VisitCallExpr(clang::CallExpr *expr) const;
    bool VisitContinueStmt(clang::ContinueStmt *stmt) const;
    bool VisitBreakStmt(clang::BreakStmt *stmt) const;

public:

    FindWaitCallVisitor () {}
    bool hasWaitCall(const clang::Stmt * rootStmt) const;
    bool hasWaitBreakCont(const clang::Stmt * rootStmt) const;
};


}

#endif //SCTOOL_SCTHREADSTMTVISITORS_H
