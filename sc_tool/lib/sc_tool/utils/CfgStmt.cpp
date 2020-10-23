/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Statement common functions
 * Author: Roman Popov
 */

#include "CfgStmt.h"

using namespace sc;

bool SubStmtVisitor::isSubStmt(const clang::Stmt *stmt, const clang::Stmt *sub) 
{
    foundSubStmt = false;
    subStmt = sub;
    this->TraverseStmt(const_cast<clang::Stmt *>(stmt));
    return foundSubStmt;
}

bool SubStmtVisitor::VisitStmt(clang::Stmt *stmt) 
{
    if (stmt == subStmt) {
        foundSubStmt = true;
        return false;
    }
    return true;
}

bool sc::isSubStmt(const clang::Stmt *stmt, const clang::Stmt *subStmt) {
    return SubStmtVisitor().isSubStmt(stmt, subStmt);
}
