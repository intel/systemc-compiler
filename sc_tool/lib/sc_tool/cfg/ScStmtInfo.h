/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SC Clang front end project.
 * 
 * Level, sub-statement and other statement information extracted from AST.
 *  
 * File:   ScStmtInfo.h
 * Author: Mikhail Moiseev
 */

#ifndef SCSTMTINFO_H
#define SCSTMTINFO_H

#include <clang/AST/RecursiveASTVisitor.h>
#include "clang/AST/Stmt.h"
#include "clang/AST/AST.h"
#include <unordered_map>
#include <unordered_set>

namespace sc {
    
/// Check binary statement ||/&& as sub-statement to get its level
class SubStmtVisitor : public clang::RecursiveASTVisitor<SubStmtVisitor> 
{
protected :
    std::unordered_map<clang::Stmt*, clang::Stmt*> subStmts;
    clang::Stmt* superStmt;
    // To check any CallExpr in sub-statements
    bool callExpr = false;
public:
    /// Required in Clang 10 to visit all AST nodes, including SC type 
    /// implicit constructors
    bool shouldVisitImplicitCode() const { return true; }
    
    SubStmtVisitor() {}
    
    /// Add all sub-statements for @stmt with super-statement @superstmt
    void addSubStmts(clang::Stmt *stmt, clang::Stmt* superstmt);

    bool VisitStmt(clang::Stmt* stmt);

    /// Get statement for which given one is sub-statement or @nullptr
    clang::Stmt* getSuperStmt(clang::Stmt* stmt) const;
    
    /// Visited sub-statements include user function call expression
    bool hasCallExpr() const;
    
    void print() const;
};

//------------------------------------------------------------------------------ 

/// This class provides levels for statements based on analysis of AST and 
/// checks statement synthesis constraints
class ScStmtInfo 
{
public:
    
    /// Run analysis for process function 
    void run(const clang::FunctionDecl* fdecl, unsigned level);
    
    /// Get statement level
    inline std::optional<unsigned> getLevel(clang::Stmt* stmt) const {
        auto i = levels.find(stmt);
        if (i != levels.end()) {
            return i->second;
        }
        return std::nullopt;
    }
    
    /// Check if statement has level, means it is not sub-statement
    inline bool hasLevel(clang::Stmt* stmt) const {
        return (levels.count(stmt) != 0);
    }
    
    inline bool isSwitchBreak(clang::Stmt* stmt) const {
        return (switchBreaks.count(stmt) != 0);
    }
    
    inline std::optional<unsigned> 
    getSubStmtLevel(clang::Stmt* stmt) const {
        if (auto ss = ssVisitor.getSuperStmt(stmt)) {
            return getLevel(ss);
        }
        return std::nullopt;
    } 
    
    /// Get statement for which given one is sub-statement or @nullptr
    clang::Stmt* getSuperStmt(clang::Stmt* stmt) const {
        return ssVisitor.getSuperStmt(stmt);
    }
    
    inline std::optional<unsigned>
    getDeclGroupLevel(clang::Stmt* stmt) const {
        using namespace clang;
        if (auto declStmt = dyn_cast<DeclStmt>(stmt)) {
            auto i = declGroups.find(declStmt->getSingleDecl());

            if (i != declGroups.end()) {
                return getLevel(i->second);
            }
        }
        return std::nullopt;
    } 
    
    /// Check if loop has function call in condition
    bool hasCallCond(clang::Stmt* stmt) const {
        return (loopCallCond.count(stmt) != 0);
    }
    
    void printLevels() const;
    void printBreaks() const;
    
protected:
    /// Statement levels
    std::unordered_map<clang::Stmt*, unsigned>  levels;
    /// Break statements in switch case/default
    std::unordered_set<clang::Stmt*>  switchBreaks;
    /// Declaration from declaration groups
    std::unordered_map<clang::Decl*, clang::Stmt*> declGroups;
    /// Loops with function call in condition
    std::unordered_set<clang::Stmt*> loopCallCond;
    /// Sub-statement visitor 
    SubStmtVisitor ssVisitor;

    /// Recursively run analysis for statement
    void analyzeStmt(clang::Stmt* stmt, unsigned level, 
                     clang::Stmt*& returnStmt, bool switchBody = false);
    
};
}

#endif /* SCSTMTINFO_H */

