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

//============================================================================

uint64_t CodeScope::id_gen = 0;

// Store general statement in the current scope
void ScScopeGraph::storeStmt(const Stmt* stmt, const string& s, bool artifIf) {
    if (artifIf) {
        artifIfStmts.insert(stmt);
    }
    currScope->push_back({stmt, s});
    //cout << "ScScopeGraph::storeStmt graph #" << hex << this << ", scope #" 
    //     << currScope.get() << ", stmt " << stmt << " : "<< dec << s << endl;
}

// Set statement as belong to METHOD with empty sensitive, 
// no tabulation used in print this statement
void ScScopeGraph::setEmptySensStmt(const Stmt* stmt) 
{
    emptySensStmt.insert(stmt);
}

// Set switch case value expression
void ScScopeGraph::storeCase(shared_ptr<CodeScope> scope, const string& s,
                             bool empty) 
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
               ((!singleBlockCThreads && isReset) ? currName : nextName) + 
               ((!singleBlockCThreads && isReset) ? NB_ASSIGN_SYM : ASSIGN_SYM) + 
               to_string(waitId) + 
               (!singleBlockCThreads ? (isReset ? "" : "; return") : "; break") +
               ((comment.empty()) ? "" : ";    // "+comment);
    currScope->push_back({stmt, s});
}

//============================================================================

// Remove given statement in the scope predecessors
void ScScopeGraph::removeStmtInPreds(std::shared_ptr<CodeScope> scope, 
                                     const Stmt* stmt, const Stmt* loopTerm, 
                                     unsigned predNum)
{
    auto i = scopePreds.find(scope);
    if (i != scopePreds.end()) {
        for (auto&& p : i->second) {
            removeStmt(p, stmt, loopTerm);
            if (predNum > 0) {
                removeStmtInPreds(p, stmt, loopTerm, predNum-1);
            }
        }
    }
}

// Remove @stmt entries from the given scope
void ScScopeGraph::removeStmt(shared_ptr<CodeScope> scope, const Stmt* stmt, 
                              const Stmt* loopTerm) 
{
    // Do not remove function if it has another current loop statement,
    // that prevent removing function call in loop condition
    if (isa<CallExpr>(stmt)) {
        auto key = pair<const Stmt*, bool>(stmt, false);
        auto p = innerScopeGraphs.find(key);
        if (p != innerScopeGraphs.end() && p->second.first != loopTerm) {
            return;
        }
    }
    
    //cout << "Remove stmt " << hex << stmt << dec << endl;
    scope->erase(remove_if(scope->begin(), scope->end(),
                         [stmt](const pair<const Stmt*, string>& obj)->bool {
                                    return (obj.first == stmt);
                                }),
                                scope->end());
}

// Remove @stmt entries from current scope and its predecessor scopes
// up to @REMOVE_STMT_PRED predecessors
void ScScopeGraph::removeStmt(const Stmt* stmt, const Stmt* loopTerm) 
{
    removeStmt(currScope, stmt, loopTerm);
    removeStmtInPreds(currScope, stmt, loopTerm, REMOVE_STMT_PRED);
}

// Check if the statement is sub-statement of another statement
bool ScScopeGraph::isSubStmt(shared_ptr<CodeScope> scope, const Stmt* stmt) 
{
    // Do not remove function where analysis started after @wait(), 
    // @wait() is in this function, so it can be called one more time after 
    auto key = pair<const Stmt*, bool>(stmt, true);
    auto p = innerScopeGraphs.find(key);
    if (p != innerScopeGraphs.end() && scope == firstScope) {
        return false;
    }
    
    return (subStmts.count(stmt) != 0);
}

// Mark @stmt as sub-statement to skip it in print
void ScScopeGraph::addSubStmt(const Stmt* stmt, const Stmt* loopTerm) 
{
    //cout << "Remove stmt #" << hex << stmt << dec << endl;
    subStmts.insert(stmt);
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
    currLevel = level;
    scopeLevel.emplace(currScope, currLevel);
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

unsigned ScScopeGraph::getCurrLevel() {
    return currLevel;
}

// Set level for the given scope @succ
void ScScopeGraph::setScopeLevel(shared_ptr<CodeScope> succ, unsigned level) {
    using namespace std;
    scopeLevel.emplace(succ, level);
    if (level > MAX_LEVEL) {
        cout << "Scope " << hex << currScope.get() << dec 
             << " has incorrect level " << level << endl;
        if (succ->empty()) {
            SCT_INTERNAL_FATAL_NOLOC("Incorrect scope level");
        } else {
            SCT_INTERNAL_FATAL(succ->back().first->getBeginLoc(), 
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
    int tabNum = INIT_LEVEL + scopeLevel.at(scope) + corr;
    // One initial tab 
    string result = TAB_SYM;
    for (int i = 0; i < tabNum; i++) {
        result += TAB_SYM;
    }
    return result;
}

string ScScopeGraph::getTabString(unsigned level) {
    // Tab number, can be negative
    int tabNum = INIT_LEVEL + level;
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

// Set initial level for function scopes
void ScScopeGraph::setInitLevel(unsigned level) {
    INIT_LEVEL = level;
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
PreparedScopes ScScopeGraph::printCurrentScope(ostream &os, 
                                               shared_ptr<CodeScope> scope, 
                                               unsigned level, bool skipDoStmt) 
{
    if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
        //cout << "<<< Start scope #" << hex << (unsigned long)scope.get() 
        //     << " in graph #" << this << dec<< endl;
        cout << "<<< Start scope #" << hex << (unsigned long)scope.get() 
             << ", " << ((scope->size()) ? scope->at(0).second : "") << dec<< endl;
        
        //if (scopeLevel.at(scope) != level) {
        //    cout << "printCurrentScope level = " << level << ", scope level = " 
        //         << scopeLevel.at(scope) << endl;
        //    assert (false);
        //}
    }
    
    // Analyzed scopes, prepared for their successors
    // Sorted by scope level, started with minimal level
    PreparedScopes prepared(scopeLevel);

    while (true) {
        // Set @scope as visited to do not take from @prepared
        visited.insert(scope);

        for (auto&& entry : *scope) {
            const Stmt* stmt = entry.first;
            string stmtStr = entry.second;
            
            // Skip sub-statements
            if (isSubStmt(scope, stmt)) continue;
            
            // Skip first @do statement in @do..@while body scope
            if (skipDoStmt) {
                SCT_TOOL_ASSERT (isa<DoStmt>(stmt), "No DO statement");
                skipDoStmt = false;
                continue;
            }
            
            if ( stmt && (isa<IfStmt>(stmt) || artifIfStmts.count(stmt)) ) {
                // Print IF statement string, no ";" after
                vector<shared_ptr<CodeScope> > succs = scopeSuccs.at(scope);
                SCT_TOOL_ASSERT (succs.size() == 2, "No two successors in IF");

                // ThenBranch scope
                std::stringstream ts;
                shared_ptr<CodeScope> thenScope = succs.at(0);
                prepared.splice(printCurrentScope(ts, thenScope, level+1));
                const string& thenStr = ts.str();

                // ElseBranch scope
                std::stringstream es;
                shared_ptr<CodeScope> elseScope = succs.at(1);
                prepared.splice(printCurrentScope(es, elseScope, level+1));
                const string& elseStr = es.str();
                
                if (!thenStr.empty() || !elseStr.empty() || !REMOVE_EMPTY_IF()) {
                
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
                
            } else 
            if ( stmt && isa<SwitchStmt>(stmt) ) {
                // Print SWITCH statement string, no ";" after
                os << getTabString(level) << stmtStr << endl;
                // Get successors, default case placed last
                vector<shared_ptr<CodeScope> > succs = scopeSuccs.at(scope);
                unordered_map<shared_ptr<CodeScope>, stringstream> succStr;
                
                // Get code string for all successors
                for (auto succ : succs) {
                    if (switchCases.count(succ)) {
                        //cout << "  " << switchCases.at(succ).first 
                        //     << " : " << switchCases.at(succ).second << endl;
                        stringstream ss;
                        prepared.splice(printCurrentScope(ss, succ, level+1));
                        succStr.emplace(succ, std::move(ss));
                    }
                }
                   
                for (auto i = succs.begin(); i != succs.end(); i++) {
                    if (switchCases.count(*i)) {
                        os << getTabString(level) << switchCases.at(*i).first 
                           << " : " << BEGIN_SYM << (switchCases.at(*i).second ? 
                              "  // Empty case without break" : "") << endl;

                        // Get first non-empty case starting with current
                        auto j = i;
                        for (; j != succs.end(); j++) {
                            if (!switchCases.at(*j).second) break;
                        }
                        
                        if (j == succs.end()) {
                            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                        ScDiag::SYNTH_SWITCH_LAST_EMPTY_CASE);
                        }

                        os << succStr.at(*j).str();
                        os << getTabString(level) << END_SYM << endl;
                    }
                }
                
                os << getTabString(level) << ENDCASE_SYM << endl;

            } else 
            if ( stmt && (isa<ForStmt>(stmt) || isa<WhileStmt>(stmt)) ) {
                // Print loop statement string, no ";" after
                vector<shared_ptr<CodeScope> > succs = scopeSuccs.at(scope);
                SCT_TOOL_ASSERT (succs.size() == 1, "No one successor in loop");

                std::stringstream ls;
                shared_ptr<CodeScope> bodyScope = succs.at(0);
                prepared.splice(printCurrentScope(ls, bodyScope, level+1));
                const string& loopStr = ls.str();
                
                if (!loopStr.empty() || !REMOVE_EMPTY_LOOP()) {
                    os << getTabString(level) << stmtStr << endl;
                    os << getTabString(level) << BEGIN_SYM << endl;
                    
                    os << loopStr;
                    
                    os << getTabString(level) << END_SYM << endl;
                }

            } else
            if ( stmt && isa<DoStmt>(stmt) ) {
                // Print @do statement string, no ";" after
                // level up as it has body block level
                std::stringstream ls;
                // @true -- skip first statement in the scope
                prepared.splice(printCurrentScope(ls, scope, level+1, true));
                const string& loopStr = ls.str();

                if (!loopStr.empty() || !REMOVE_EMPTY_LOOP()) {
                    os << getTabString(level) << "do" << endl;
                    os << getTabString(level) << BEGIN_SYM << endl;

                    os << loopStr;

                    os << getTabString(level) << END_SYM << endl;
                    os << getTabString(level) << stmtStr << ";" << endl;
                }
                
                // Skip print the scope other statements, already printed inside
                // Next scope will be after @DoStmt
                break;

            } else {
                // Print statement string, empty string can be for function call
                // Commented: do not print parameters for record constructor as that already
                // done before print of initialization list
                if (!stmtStr.empty() /*&& (!stmt || !isa<CXXConstructExpr>(stmt))*/) {
                    // Split and print one or more lines with tabulation
                    bool noTabStmt = emptySensStmt.count(stmt);
                    printSplitString(os, stmtStr, noTabStmt ? "" : 
                                     getTabString(level));
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
                        fcallScope->setInitLevel(level + INIT_LEVEL);
                        fcallScope->printCurrentScope(fs, fcallScope->
                                                      getFirstScope(), 0);
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
            cout << "--- Scope " << hex << (unsigned long)scope.get() << " finished" << dec<< endl;
        }    
        // Add this scope as prepared to its successors
        // Not used for non-empty IF branches as they added into @visited before
        // Required for empty IF branches
        if (scopeSuccs.count(scope)) {
            //cout << "Scope " << scope->asString() << endl;
            for (auto&& succ : scopeSuccs.at(scope)) {
                //cout << " succ " << succ->asString() << endl;
                auto i = prepared.rbegin();
                for (; i != prepared.rend(); ++i) {
                    if (i->first == succ) {
                        vector<shared_ptr<CodeScope> >& preds = i->second;
                        preds.push_back(scope);
                        break;
                    }
                }
                // Insert new scope according with its level
                if (i == prepared.rend()) {
                    // Insert successor in according with its scope level 
                    // Level of block after loop is less than level of blocks 
                    // in the loop body, so loop body analyzed first
                    unsigned succLevel = scopeLevel.at(succ);
                    auto j = prepared.begin();
                    for (; j != prepared.end(); ++j) {
                        if (succLevel < scopeLevel.at(j->first)) {
                            break;
                        }
                    }
                    // Insert new scope before element pointed by @j
                    prepared.insert(j, {succ, 
                                    vector<shared_ptr<CodeScope> >(1, scope)});
                }
            }
        }

        // Get next scope with maximal scope level
        shared_ptr<CodeScope> next = nullptr;
        auto i = prepared.rbegin();
        for (; i != prepared.rend(); ++i) {
            // Check all next scope predecessors are analyzed and 
            // scope was not visited before
            if (i->second.size() == scopePreds.at(i->first).size() &&
                visited.count(i->first) == 0) 
            {
                next = i->first;
                break;
            }
        }

        // If not all next scope predecessors analyzed or next scope level
        // is less than current level, return from this function
        if (next == nullptr || scopeLevel.at(next) < level) {
            if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
                cout << "--- Scope up, as next " << ((next) ? "level up" : "is null") << endl;
            }
            return prepared;
        }
        // Go to the next scope
        scope = next;
        if (DebugOptions::isEnabled(DebugComponent::doScopeGraph)) {
            cout << "--- Next scope " << hex << (unsigned long)scope.get() << dec<< endl;
        }
    }
}

void ScScopeGraph::clearAfterPrint() 
{
    visited.clear();
}

void ScScopeGraph::printAllScopes(ostream &os) 
{
    printCurrentScope(os, getFirstScope(), 0);
    clearAfterPrint();
}

// Clone scope graph at wait()
std::unique_ptr<ScScopeGraph> 
ScScopeGraph::clone(std::shared_ptr<ScScopeGraph> innerGraph) 
{
    auto res = std::make_unique<ScScopeGraph>(noreadyBlockAllowed);

    auto scope = make_shared<CodeScope>();
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