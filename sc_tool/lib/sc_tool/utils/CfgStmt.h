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

#ifndef CFGSTMT_H
#define CFGSTMT_H

#include <clang/AST/RecursiveASTVisitor.h>
#include "clang/AST/Stmt.h"

namespace sc {

/// Check sub-statement in statement visitor
class SubStmtVisitor : public clang::RecursiveASTVisitor<SubStmtVisitor> 
{
private:
    const clang::Stmt *subStmt = nullptr;
    bool foundSubStmt = false;
    
public:

    SubStmtVisitor() {}

    bool isSubStmt(const clang::Stmt *stmt, const clang::Stmt *sub);

    bool VisitStmt(clang::Stmt *stmt);
};

/// Check sub-statement in statement
bool isSubStmt(const clang::Stmt *stmt, const clang::Stmt *subStmt);

}
#endif /* CFGSTMT_H */

