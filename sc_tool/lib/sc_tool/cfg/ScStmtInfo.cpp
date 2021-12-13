/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "sc_tool/cfg/ScStmtInfo.h"
#include "sc_tool/cfg/ScTraverseCommon.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include <iostream>

using namespace sc;
using namespace clang;
using std::cout; using std::endl; using std::hex; using std::dec;

// Add all sub-statements for @stmt with super-statement @superstmt
void SubStmtVisitor::addSubStmts(clang::Stmt* stmt, clang::Stmt* superstmt)
{
    callExpr = false;
    superStmt = superstmt;
    this->TraverseStmt(stmt);
}

bool SubStmtVisitor::VisitStmt(clang::Stmt* stmt) 
{
    if (stmt != superStmt) {
        subStmts.emplace(stmt, superStmt);
    }
    if (isUserCallExpr(stmt)) {
        callExpr = true;
    }
    return true;
}

clang::Stmt* SubStmtVisitor::getSuperStmt(clang::Stmt* stmt) const
{
    auto i = subStmts.find(stmt);
    if (i != subStmts.end()) {
        return i->second;
    }
    return nullptr;
}

bool SubStmtVisitor::hasCallExpr() const
{
    return callExpr;
}

void SubStmtVisitor::print() const
{
    cout << "SubStmtVisitor : " << endl;
    for (auto& entry : subStmts) {
        cout << " " << hex << entry.first << " " << entry.second << dec << endl;
    }
}

//-----------------------------------------------------------------------------

// Run analysis for process function 
void ScStmtInfo::run(const clang::FunctionDecl* fdecl, unsigned level) 
{
    using namespace std;
    
    if (fdecl) {
        clang::Stmt* returnStmt = nullptr;
        analyzeStmt(fdecl->getBody(), level, returnStmt);
    }
    //cout << "-------------------------------------------------" << endl;
}

// Recursively run analysis for statement
void ScStmtInfo::analyzeStmt(clang::Stmt* stmt, unsigned level, 
                             clang::Stmt*& returnStmt, bool switchBody)
{
    using namespace std;
    
    if (stmt == nullptr) return;
    
    if (CompoundStmt* compStmt = dyn_cast<CompoundStmt>(stmt)) {
        //cout << "----- Compound #" << hex << stmt << dec << endl;
        //stmt->dumpColor();
        
        if (switchBody) {
            // Join return flags from all the switch cases
            clang::Stmt* returnSwitch = nullptr;
            for (Stmt* s : compStmt->body()) {
                clang::Stmt* returnCase = returnStmt;
                analyzeStmt(s, level, returnCase, switchBody);
                returnSwitch = returnSwitch ? returnSwitch : returnCase;
            }
            returnStmt = returnStmt ? returnStmt : returnSwitch;
            
        } else {
            for (Stmt* s : compStmt->body()) {
                analyzeStmt(s, level, returnStmt, switchBody);
            }
        }
    } else {
        stmt = removeExprCleanups(stmt);
        //cout << "----- Stmt #" << hex << stmt << dec << " level " << level << endl;
        //stmt->dumpColor();
        
        if (returnStmt) {
            ScDiag::reportScDiag(returnStmt->getBeginLoc(), 
                                 ScDiag::SYNTH_CODE_AFTER_RETURN);
        }
        
        if (IfStmt* ifStmt = dyn_cast<IfStmt>(stmt)) {
            clang::Stmt* returnElse = returnStmt;
            analyzeStmt(ifStmt->getThen(), level+1, returnStmt, switchBody);
            analyzeStmt(ifStmt->getElse(), level+1, returnElse, switchBody);
            ssVisitor.addSubStmts(ifStmt->getCond(), ifStmt);
            returnStmt = returnStmt ? returnStmt : returnElse;

        } else 
        if (SwitchStmt* switchStmt = dyn_cast<SwitchStmt>(stmt)) {
            //cout << "----- Compound Switch Body #" << hex << switchStmt->getBody() << dec << endl;
            //switchStmt->getBody()->dumpColor();
            
            analyzeStmt(switchStmt->getBody(), level+1, returnStmt, true);
            ssVisitor.addSubStmts(switchStmt->getCond(), switchStmt);

        } else 
        if (SwitchCase* caseStmt = dyn_cast<SwitchCase>(stmt)) {
            analyzeStmt(caseStmt->getSubStmt(), level, returnStmt, switchBody);

        } else 
        if (ForStmt* forStmt = dyn_cast<ForStmt>(stmt)) {
            analyzeStmt(forStmt->getBody(), level+1, returnStmt, false);
            
            ssVisitor.addSubStmts(forStmt->getCond(), forStmt);
            if (ssVisitor.hasCallExpr()) {
                loopCallCond.insert(stmt);
            }
            
            ssVisitor.addSubStmts(forStmt->getInc(), forStmt);
            if (ssVisitor.hasCallExpr()) {
                ScDiag::reportScDiag(forStmt->getInc()->getBeginLoc(), 
                                     ScDiag::SYNTH_FUNC_CALL_II_LOOP);
            }
            
            ssVisitor.addSubStmts(forStmt->getInit(), forStmt);
            if (ssVisitor.hasCallExpr()) {
                ScDiag::reportScDiag(forStmt->getInit()->getBeginLoc(), 
                                     ScDiag::SYNTH_FUNC_CALL_II_LOOP);
            }

        } else 
        if (WhileStmt* whileStmt = dyn_cast<WhileStmt>(stmt)) {
            analyzeStmt(whileStmt->getBody(), level+1, returnStmt, false);
            ssVisitor.addSubStmts(whileStmt->getCond(), whileStmt);
            if (ssVisitor.hasCallExpr()) {
                loopCallCond.insert(stmt);
            }

        } else 
        if (DoStmt* doStmt = dyn_cast<DoStmt>(stmt)) {
            analyzeStmt(doStmt->getBody(), level+1, returnStmt, false);
            ssVisitor.addSubStmts(doStmt->getCond(), doStmt);
            if (ssVisitor.hasCallExpr()) {
                loopCallCond.insert(stmt);
            }
            
        } else 
        if (DeclStmt* declStmt = dyn_cast<DeclStmt>(stmt)) {
            // Declaration group is transformed into individual declaration 
            // statements in CFG elements, so declarations are registered
            if (!declStmt->isSingleDecl()) {
                for (auto decl : declStmt->getDeclGroup()) {
                    declGroups.emplace(decl, declStmt);
                }
            }
            ssVisitor.addSubStmts(declStmt, declStmt);
            
        } else 
        if (LabelStmt* labelStmt = dyn_cast<LabelStmt>(stmt)) {
            // Ignore labels
            analyzeStmt(labelStmt->getSubStmt(), level, returnStmt, switchBody);
            
        } else {
            // Register break in switch body
            if (switchBody && isa<BreakStmt>(stmt)) {
                switchBreaks.insert(stmt);
            }
            
            if (!returnStmt && isa<ReturnStmt>(stmt)) {
                returnStmt = stmt;
            }
            ssVisitor.addSubStmts(stmt, stmt);
        }

        auto i = levels.emplace(stmt, level);
        if (!i.second) {
            i.first->second = level;
        }
    }
}


// Print all statement levels
void ScStmtInfo::printLevels() const
{
    using namespace std;
    cout << "ScStmtInfo levels -------------------- " << endl;
    for (auto i : levels) {
        cout << "   " << hex << i.first << dec << " : " << i.second << endl;
    }
}

void ScStmtInfo::printBreaks() const {
    using namespace std;
    cout << "ScStmtInfo breaks -------------------- " << endl;
    for (auto s : switchBreaks) {
        cout << "   " << hex << s << dec << endl;
    }
}
