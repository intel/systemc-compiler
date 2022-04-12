/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/scope/ScScopeGraph.h"
#include "sc_tool/cfg/ScTraverseCommon.h"
#include "sc_tool/utils/StringFormat.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/ScCommandLine.h"

#include "clang/AST/ExprCXX.h"
#include "clang/AST/Expr.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace clang;
using namespace llvm;

namespace sc {

/// Statement assigned variable, used to remove variable initialization 
/// statements for removed variables/constants
std::unordered_map<const clang::Stmt*, SValue> ScScopeGraph::stmtAssignVars;

//============================================================================

uint64_t CodeScope::id_gen = 1;

// Store general statement in the current scope
void ScScopeGraph::storeStmt(const Stmt* stmt, const string& s, bool artifIf) {
    if (artifIf) {
        artifIfStmts.insert(stmt);
    }
    currScope->push_back({stmt, s});
    //cout << "ScScopeGraph::storeStmt graph #" << hex << this << ", scope #" 
    //     << currScope.get() << ", stmt " << stmt << " : "<< dec << s << endl; 
}

// Add comment for statement
void ScScopeGraph::addComment(const Stmt* stmt, const string& comment) {
    stmtComments.emplace(stmt, comment);
}

// Store @sct_assert statement in the current scope
void ScScopeGraph::storeAssertStmt(const Stmt* stmt, const string& s) {
    assertStmts.insert(stmt);
    currScope->push_back({stmt, s});
}

void ScScopeGraph::storeNullStmt(std::shared_ptr<CodeScope> scope) {
    scope->push_back({nullptr, ""});
}

// Set statement as belong to METHOD with empty sensitive, 
// no tabulation used in print this statement
void ScScopeGraph::setEmptySensStmt(const Stmt* stmt) 
{
    emptySensStmt.insert(stmt);
}

// Set switch case value expression
/// \param empty -- case is empty (no statements and no break)
void ScScopeGraph::storeCase(shared_ptr<CodeScope> scope, 
                             const string& s, bool empty) 
{
    switchCases.emplace(scope, make_pair(s, empty));
    
    if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
        cout << "setSwitchCase scope " << hex << scope.get() << dec << " : " 
             << s << " empty " << empty << endl;
    }
}

// Store THREAD state variable assignment next state value
void ScScopeGraph::storeStateAssign(const clang::Stmt* stmt, 
                                    const std::pair<std::string,std::string>& names, 
                                    size_t waitId, bool isReset, bool addTab,
                                    const std::string& comment)
{
    string currName = names.first.empty() ? STATE_VAR_NAME : names.first;
    string nextName = names.second.empty() ? STATE_VAR_NAME : names.second;
    
    string s = (addTab ? "    " : "") + 
               ((isReset) ? currName : nextName) + 
               ((isReset) ? NB_ASSIGN_SYM : ASSIGN_SYM) + 
               to_string(waitId) + 
               (isReset ? "" : "; return") +
               ((comment.empty()) ? "" : ";    // "+comment);
    currScope->push_back({stmt, s});
}

}

//============================================================================

namespace std {
    
std::size_t hash< std::pair<const clang::Stmt*, bool> >::operator () 
    (const std::pair<const clang::Stmt*, bool>& obj) const 
{
    using std::hash;
    return (std::hash<const void*>()(obj.first) ^ std::hash<bool>()(obj.second));
}
}

//=========================================================================

namespace sc {

// Set/get current scope
void ScScopeGraph::setFirstScope(shared_ptr<CodeScope> scope) {
    firstScope = scope;
}
shared_ptr<CodeScope> ScScopeGraph::getFirstScope() {
    return firstScope;
}

// Set/get current scope
void ScScopeGraph::setCurrScope(shared_ptr<CodeScope> scope) {
    currScope = scope;
}
shared_ptr<CodeScope> ScScopeGraph::getCurrScope() {
    return currScope;
}

// Set/get current block level as maximal of predecessors
void ScScopeGraph::setCurrLevel(unsigned level) {
    using namespace std;

    currScope->setLevel(level);
    
    if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
        cout << "Scope " << hex << currScope.get() << dec << " set level " << level << endl;
    }
    
    if (level > MAX_LEVEL) {
        cout << "Scope " << hex << currScope.get() << dec 
             << " has incorrect level " << level << endl;
        if (currScope->empty()) {
            SCT_INTERNAL_FATAL_NOLOC("Incorrect current scope level");
        } else {
            SCT_INTERNAL_FATAL(currScope->back().first->getBeginLoc(), 
                             "Incorrect current scope level");
        }
    }
}

// Get original level as used in TraverseProc, no correction by InMainLoop
unsigned ScScopeGraph::getCurrLevel() {
    return currScope->getLevel();
}

// Set level for the given scope 
void ScScopeGraph::setScopeLevel(shared_ptr<CodeScope> scope, unsigned level) {
    using namespace std;
    
    scope->setLevel(level);

    if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
        cout << "Scope " << hex << currScope.get() << dec << " set level " << level << endl;
    }
    
    if (level > MAX_LEVEL) {
        cout << "Scope " << hex << currScope.get() << dec 
             << " has incorrect level " << level << endl;
        if (scope->empty()) {
            SCT_INTERNAL_FATAL_NOLOC("Incorrect scope level");
        } else {
            SCT_INTERNAL_FATAL(scope->back().first->getBeginLoc(), 
                             "Incorrect scope level");
        }
    }
}

// Add @succ to successors of @currScope
void ScScopeGraph::addScopeSucc(shared_ptr<CodeScope> succ) {
    using namespace std;
    if (scopeSuccs.count(currScope) == 0) {
        scopeSuccs.emplace(currScope, vector<shared_ptr<CodeScope> >(1, succ));
    } else {
        vector<shared_ptr<CodeScope> >& succs = scopeSuccs.at(currScope);
        succs.push_back(succ);
    }
}

// Add @succ to successors of @scope
void ScScopeGraph::addScopeSucc(shared_ptr<CodeScope> scope, 
                                shared_ptr<CodeScope> succ) {
    using namespace std;
    if (scopeSuccs.count(scope) == 0) {
        scopeSuccs.emplace(scope, vector<shared_ptr<CodeScope> >(1, succ));
    } else {
        vector<shared_ptr<CodeScope> >& succs = scopeSuccs.at(scope);
        succs.push_back(succ);
    }
}

// Add @scope to @currScope predecessors
void ScScopeGraph::addScopePred(shared_ptr<CodeScope> pred) {
    using namespace std;
    if (scopePreds.count(currScope) == 0) {
        scopePreds.emplace(currScope, vector<shared_ptr<CodeScope> >(1, pred));
    } else {
        vector<shared_ptr<CodeScope> >& preds = scopePreds.at(currScope);
        preds.push_back(pred);
    }
}

// Add @scope to @currScope predecessors
void ScScopeGraph::addScopePred(shared_ptr<CodeScope> scope, 
                                shared_ptr<CodeScope> pred) {
    using namespace std;
    if (scopePreds.count(scope) == 0) {
        scopePreds.emplace(scope, vector<shared_ptr<CodeScope> >(1, pred));
    } else {
        vector<shared_ptr<CodeScope> >& preds = scopePreds.at(scope);
        preds.push_back(pred);
    }
}

//=========================================================================

// Get tabulation string with length corresponds scope depth
// \corr -- tab number correction, can be negative
string ScScopeGraph::getTabString(shared_ptr<CodeScope> scope, int corr) {
    // Tab number, can be negative
    int tabNum = scope->getCorrLevel() + corr;
    // One initial tab 
    string result = TAB_SYM;
    for (int i = 0; i < tabNum; i++) {
        result += TAB_SYM;
    }
    return result;
}

string ScScopeGraph::getTabString(unsigned level) {
    // Tab number, can be negative
    int tabNum = level;
    // One initial tab 
    string result = TAB_SYM;
    for (int i = 0; i < tabNum; i++) {
        result += TAB_SYM;
    }
    return result;
}

/// Set function name
void ScScopeGraph::setName(const string& fname, bool fcall) {
    name = fname;
    funcCall = fcall;
}

// Add scope graph for function call
void ScScopeGraph::addFCallScope(const Stmt* stmt, const Stmt* loopTerm,
                                 shared_ptr<ScScopeGraph> graph) 
{
    auto key = pair<const Stmt*, bool>(stmt, false);
    // Can be called multiple times for the same call statement in CTHREAD
    // function call in several loops, first call stored and used for all
    innerScopeGraphs.emplace(key, std::pair<const clang::Stmt*, 
                             std::shared_ptr<ScScopeGraph>>(loopTerm, graph));
    lastFuncCall = stmt;
}

// Print scope statements
// \param printLevel -- level for output tabs
// \param noExitByLevel -- no return from this printCurrentScope() by level up 
PreparedScopes ScScopeGraph::printCurrentScope(ostream &os, 
                                shared_ptr<CodeScope> scope, unsigned level, 
                                bool noExitByLevel) 
{
    // Level for out from this printCurrentScope()
    unsigned scopeLevel = scope->getCorrLevel();
    
    if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
        cout << "<<< Start scope #" << hex << (unsigned long)scope.get() 
             << " in graph #" << this << ", noExitByLevel " << noExitByLevel 
             << dec << ", " << ((scope->size()) ? scope->at(0).second : "")
             << " level " << scopeLevel << "/" << level 
             << ", isMainLoop " << scope->isMainLoop() << endl;
        
        //if (scope->level != level) {
        //    cout << "printCurrentScope level = " << level << ", scope level = " 
        //         << scope->getCorrLevel() << endl;
        //    assert (false);
        //}
    }
    
    // Analyzed scopes, prepared for their successors
    // Sorted by scope level, started with minimal level
    PreparedScopes prepared; 

    while (true) {
        // Set @scope as visited to do not take from @prepared
        visited.insert(scope);

        for (auto&& entry : *scope) {
            const Stmt* stmt = entry.first;
            string stmtStr = entry.second;
            
            if ( stmt && (isa<IfStmt>(stmt) || artifIfStmts.count(stmt)) ) {
                // Print IF statement string, no ";" after
                vector<shared_ptr<CodeScope> > succs = scopeSuccs.at(scope);
                SCT_TOOL_ASSERT (succs.size() == 2, "No two successors in IF");
                shared_ptr<CodeScope> thenScope = succs.at(0);
                shared_ptr<CodeScope> elseScope = succs.at(1);

                // Remove IF statement, only Then or Else branch printed
                bool removeIf = (thenScope->isDead() || elseScope->isDead()) && 
                                REMOVE_EMPTY_IF();
                
                // ThenBranch scope
                std::stringstream ts;
                if (!thenScope->isDead()) {
                    prepared.splice(printCurrentScope(ts, thenScope, 
                                    removeIf ? level : level+1));
                }
                const string& thenStr = (!thenScope->isDead()) ? ts.str() : "";

                // ElseBranch scope
                std::stringstream es;
                if (!elseScope->isDead()) {
                    prepared.splice(printCurrentScope(es, elseScope, 
                                    removeIf ? level : level+1));
                }
                const string& elseStr = (!elseScope->isDead()) ? es.str() : "";
                
                if (!thenStr.empty() || !elseStr.empty() || !REMOVE_EMPTY_IF()){

                    if (removeIf) {
                         if (thenScope->isDead()) os << elseStr;
                         else os << thenStr;

                    } else {
                        os << getTabString(level) << stmtStr << endl;
                        os << getTabString(level) << BEGIN_SYM << endl;
                        os << thenStr;

                        if (!elseStr.empty()) {
                            os << getTabString(level) << END_SYM << " else " 
                               << BEGIN_SYM << endl;
                            os << elseStr;
                        }
                        os << getTabString(level) << END_SYM << endl;
                    }
                }
                
            } else 
            if ( stmt && isa<SwitchStmt>(stmt) ) {
                // Print SWITCH statement string, no ";" after
                //cout << "CASE " << stmtStr << endl;
                // Get successors, default case placed last
                vector<shared_ptr<CodeScope> > succs = scopeSuccs.at(scope);
                unordered_map<shared_ptr<CodeScope>, stringstream> succStr;
                // Remove switch if there is no alive cases
                bool removeSwitch = true;
                        
                // Get code string for all successors
                for (auto succ : succs) {
                    if (succ->isDead()) continue;
                    
                    //cout << "   scope " << hex << succ << dec << endl;
                    auto i = switchCases.find(succ);
                    if (i != switchCases.end()) {
                        stringstream ss;
                        // Get string for non-empty case only, empty not analyzed
                        if (!i->second.second) {
                            prepared.splice(printCurrentScope(ss, succ, level+1));
                        }
                        succStr.emplace(succ, std::move(ss));
                        removeSwitch = false;
                        //cout << "  " << i->second.first  << " : " << i->second.second << endl;
                    }
                }
                
                // Print @switch keyword    
                if (!removeSwitch) os << getTabString(level) << stmtStr << endl;
                
                // Print cases
                for (auto i = succs.begin(); i != succs.end(); ++i) {
                    if ((*i)->isDead()) continue;
                    
                    auto k = switchCases.find(*i);
                    if (k != switchCases.end()) {
                        os << getTabString(level) << k->second.first << " : " 
                           << BEGIN_SYM 
                           << (k->second.second ? "  // Empty case without break" : "") 
                           << endl;

                        // Get first non-empty case starting with current
                        auto j = i;
                        for (; j != succs.end(); j++) {
                            auto l = switchCases.find(*j);
                            if (l != switchCases.end()) {
                                if (!l->second.second) break;
                            }
                        }

                        if (j != succs.end()) {
                            os << succStr.at(*j).str();
                        } else {
                            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                                 ScDiag::SYNTH_SWITCH_LAST_EMPTY_CASE);
                        }
                        os << getTabString(level) << END_SYM << endl;
                    }
                }
                
                //cout << "ENDCASE " << stmtStr << endl;
                if (!removeSwitch) os << getTabString(level) << ENDCASE_SYM << endl;

            } else 
            if ( stmt && (isa<ForStmt>(stmt) || isa<WhileStmt>(stmt) || 
                          isa<DoStmt>(stmt)) ) {
                // Print loop statement string, no ";" after
                //cout << "LOOP #" << hex << stmt << dec << endl;
                vector<shared_ptr<CodeScope> > succs = scopeSuccs.at(scope);
                SCT_TOOL_ASSERT (succs.size() == 1, "No one successor in loop");

                std::stringstream ls;
                shared_ptr<CodeScope> bodyScope = succs.at(0);
                prepared.splice(printCurrentScope(ls, bodyScope, level+1));
                const string& loopStr = ls.str();
                
                // Do not remove FOR loop with external counter as 
                // it can be used after the loop
                if (!loopStr.empty() || !REMOVE_EMPTY_LOOP() ||
                    hasForExtrCntr(const_cast<Stmt*>(stmt)))
                {
                    if (isa<DoStmt>(stmt)) {
                        os << getTabString(level) << "do" << endl;
                    } else {
                        os << getTabString(level) << stmtStr << endl;
                    }
                    os << getTabString(level) << BEGIN_SYM << endl;
                    
                    os << loopStr;
                    
                    os << getTabString(level) << END_SYM << endl;
                    if (isa<DoStmt>(stmt)) {
                        os << getTabString(level) << stmtStr << ";" << endl;
                    }
                }
                //cout << "ENDLOOP #" << hex << stmt << dec << endl;

            } else 
            if ( stmt && assertStmts.count(stmt) ) {
                // @sct_assert
                bool noTabStmt = emptySensStmt.count(stmt);
                string tabStr = noTabStmt ? "" : getTabString(level);
                
                os << tabStr << "`ifndef INTEL_SVA_OFF" << endl;
                os << tabStr << TAB_SYM << stmtStr << ";" << endl;
                os << tabStr << "`endif // INTEL_SVA_OFF" << endl;
                
                if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
                    cout << "   " << stmtStr << endl;
                }
                
            } else {
                // Print statement string, empty string can be for function call
                // Commented: do not print parameters for record constructor as 
                // that already done before print of initialization list
                if (!stmtStr.empty() /*&& (!stmt || !isa<CXXConstructExpr>(stmt))*/) {
                    // Split and print one or more lines with tabulation
                    bool noTabStmt = emptySensStmt.count(stmt);
                    std::string comment;
                    auto i = stmtComments.find(stmt);
                    if (i != stmtComments.end()) {
                        comment = "// "+i->second;
                    }
                    printSplitString(os, stmtStr, 
                                     noTabStmt ? "" : getTabString(level), 
                                     comment);
                    
                    if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
                        cout << "   " << stmtStr << endl;
                    }
                }
                
                // For function call/break&continue check for function body
                // Break&continue may have no function body
                // @DeclStmt for record declaration contain constructor call
                if (stmt && (isa<CallExpr>(stmt) || isa<DeclStmt>(stmt) || 
                             isa<BreakStmt>(stmt) || isa<ContinueStmt>(stmt))) 
                {
                    // At wait() call, current scope is first scope 
                    // @waitFlag lets to distinguish exit from a method with 
                    // @wait() (true) and enter to the same method (false)
                    bool waitFlag = scope == firstScope && isa<CallExpr>(stmt);
                    auto key = pair<const Stmt*, bool>(stmt, waitFlag);
                    bool scopeFound = innerScopeGraphs.count(key);
                    
                    // Check if normal function call in first scope, it has
                    // different statement, so try to find it with false in key
                    if (!scopeFound && waitFlag) {
                        waitFlag = false;
                        key.second = waitFlag;
                        scopeFound = innerScopeGraphs.count(key);
                    }
                    
                    // Print function body
                    if ( scopeFound ) {
                        auto fcallScope = innerScopeGraphs.at(key).second;
                        //cout << "Fcall fcallScope" << hex << fcallScope.get() << dec << ", level " << level << endl;
                        std::stringstream fs;
                        // Prohibit to exit from @printCurrentScope() by up level, 
                        // required to level up from wait()/break/continue
                        fcallScope->printCurrentScope(
                                fs, fcallScope->getFirstScope(), level, true);
                        fcallScope->clearAfterPrint();
                        const string& funcStr = fs.str();
                        
                        // Print function if its body not empty
                        if (!funcStr.empty() || !REMOVE_EMPTY_FUNC()) {

                            if (fcallScope->funcCall) {
                                os << getTabString(level) << "// Call " 
                                   << fcallScope->name << "() begin" << endl;
                            } else {
                                os << getTabString(level) << "// " 
                                   << fcallScope->name << " begin"<< endl;
                            }
                            
                            os << funcStr;

                            if (fcallScope->funcCall) {
                                os << getTabString(level) << "// Call " 
                                   << fcallScope->name << "() end" << endl;
                            } else {
                                os << getTabString(level) << "// " 
                                   << fcallScope->name << " end" << endl;
                            }
                        }
                    }
                }
            }
        }

        if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
            cout << "--- Scope " << hex << (unsigned long)scope.get() << dec
                 << " finished, level " << scope->getCorrLevel() 
                 << " empty " << scope->empty()  << endl;
        }    
        // Add this scope as prepared to its successors
        // Not used for non-empty IF branches as they added into @visited before
        // Required for empty IF branches
        if (scopeSuccs.count(scope)) {
            //cout << "Scope " << scope->asString() << endl;
            for (auto&& succ : scopeSuccs.at(scope)) {
                // Skip dead scope corresponded to dead code
                if (succ->isDead()) continue;
                
                //cout << " succ " << succ->asString() << endl;
                auto i = prepared.rbegin();
                for (; i != prepared.rend(); ++i) {
                    if (i->first == succ) {
                        i->second.push_back(scope);
                        break;
                    }
                }
                // Insert new scope according with its level
                if (i == prepared.rend()) {
                    // Insert successor in according with its scope level 
                    // Level of block after loop is less than level of blocks 
                    // in the loop body, so loop body analyzed first
                    //cout << "Scope get level #" << hex << succ << dec << endl;
                    unsigned succLevel = succ->getCorrLevel();
                    auto j = prepared.begin();
                    for (; j != prepared.end(); ++j) {
                        if (succLevel < j->first->getCorrLevel()) {
                            break;
                        }
                    }
                    // Insert new scope before element pointed by @j
                    prepared.insert(j, {succ, vector<shared_ptr<CodeScope>>(1, scope)});
                }
            }
        }

        // Get next scope with maximal scope level
        shared_ptr<CodeScope> next = nullptr;
        auto i = prepared.rbegin();
        for (; i != prepared.rend(); ++i) {
            // Check all next scope predecessors are analyzed, required for 
            // conditional operator and break/continue correct order print
            if (i->second.size() == scopePreds.at(i->first).size() &&
                visited.count(i->first) == 0) 
            {
                next = i->first;
                break;
            }
        }

        // If not all next scope predecessors analyzed or next scope level
        // is less than current level, return from this function
        if (next == nullptr || (!next->empty() && 
            next->getCorrLevel() < scopeLevel && !noExitByLevel)) 
        {
            if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
                cout << "--- Scope up, as next " << ((next) ? "level up" : "is null") << endl;
                if (next) cout << "    next " << hex << (size_t)next.get() << dec << endl;
            }
            return prepared;
        }
        // Go to the next scope
        scope = next;
        if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
            cout << "--- Next scope " << hex << (unsigned long)scope.get() << dec
                  << " level " << scope->getCorrLevel() << endl;
        }
    }
}

void ScScopeGraph::clearAfterPrint() 
{
    visited.clear();
}

void ScScopeGraph::printAllScopes(ostream &os) 
{
    if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
        cout << endl << "------------ printAllScopes --------------" << endl;
    }
    printCurrentScope(os, getFirstScope(), 0, true);
    clearAfterPrint();
}

// Clone scope graph at break/continue/wait()
std::unique_ptr<ScScopeGraph> 
ScScopeGraph::clone(std::shared_ptr<ScScopeGraph> innerGraph, bool inMainLoop) 
{
    auto res = std::make_unique<ScScopeGraph>();

    auto scope = make_shared<CodeScope>(0, inMainLoop);
    res->firstScope = scope;
    res->currScope = scope;
    res->setCurrLevel(0);
    res->setName(name, funcCall);
    // Store last call to use it in next clone(), required if wait() is in 
    // internal function
    res->lastFuncCall = lastFuncCall;   

    // Add last function call and scope graph to provide correct print
    if (innerGraph) {
        // Use @true at wait() call
        auto key = pair<const Stmt*, bool>(lastFuncCall, true);
        // Use @nullptr instead loop statement -- not important as this function
        // call statement will not be removed anyway
        res->innerScopeGraphs.emplace(key, std::pair<const clang::Stmt*, 
                        std::shared_ptr<ScScopeGraph>>(nullptr, innerGraph));
        // Add empty string for function call statement into current scope
        res->storeStmt(lastFuncCall, string());    
    }
    
    return res;
}

}