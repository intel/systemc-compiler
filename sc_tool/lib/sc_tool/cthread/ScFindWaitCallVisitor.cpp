/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "ScFindWaitCallVisitor.h"

using namespace clang;
using namespace llvm;

namespace sc
{

bool FindWaitCallVisitor::VisitCallExpr(clang::CallExpr *expr) const {
    if (searchWait) {
        const auto * funcDecl = expr->getDirectCallee();

        if (isWaitDecl(funcDecl)) {
            foundStmt = true;
            return false;
        }

        if (!isStdFuncDecl(funcDecl)) {
            FindWaitCallVisitor callVisitor{};
            if (callVisitor.hasWaitCall(funcDecl->getBody())) {
                foundStmt = true;
                return false;
            }
        }
    }
    return true;
}

bool FindWaitCallVisitor::VisitContinueStmt(clang::ContinueStmt *stmt) const {
    if (searchContinue) {
        foundStmt = true;
        return false;
    }

    return true;
}

bool FindWaitCallVisitor::VisitBreakStmt(clang::BreakStmt *stmt) const {
    if (searchBreak) {
        foundStmt = true;
        return false;
    }
    return true;
}

bool FindWaitCallVisitor::hasWaitCall(const clang::Stmt *rootStmt) const {

    auto cached = hasWaitMap.find(rootStmt);
    if (cached != end(hasWaitMap))
        return cached->second;

    searchWait = true;
    foundStmt = false;

    const_cast<FindWaitCallVisitor*>(this)->TraverseStmt(
        const_cast<clang::Stmt *>(rootStmt));

    hasWaitMap[rootStmt] = foundStmt;
    searchWait = false;
    return foundStmt;
}

bool FindWaitCallVisitor::hasWaitBreakCont(const clang::Stmt *rootStmt) const {
    searchWait = true;
    searchBreak = true;
    searchContinue = true;
    foundStmt = false;

    const_cast<FindWaitCallVisitor*>(this)->TraverseStmt(
        const_cast<clang::Stmt *>(rootStmt));

    searchWait = false;
    searchBreak = false;
    searchContinue = false;

    return foundStmt;
}

} // end namespace scl