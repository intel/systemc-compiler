/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/* 
 * SC scope graph base class for ScScopeAnalyzer and ScVerilogGenerator
 *
 * Author: Mikhail Moiseev
 */

#ifndef SCSCOPEGRAPH_H
#define SCSCOPEGRAPH_H

#include "sc_tool/cfg/SValue.h"
#include <clang/AST/Stmt.h>

#include <unordered_set>
#include <unordered_map>
#include <list>
#include <vector>
#include <iostream>


namespace sc {

/// Code scope contains statements in order and they string representations
class CodeScope : public std::vector<std::pair<const clang::Stmt*, std::string> >  
{
protected:
    static uint64_t id_gen;
    /// Scope unique ID
    uint64_t id;
    /// Original level as in TraverseProc, can be decreased by 1 inside ScopeGraph 
    unsigned level;
    /// Inside of main loop of CTHREAD process, level decreased by 1 
    const bool inMainLoop;
    
public:
    
    CodeScope() : 
        id(0), level(0), inMainLoop(false)
    {}
        
    CodeScope(unsigned level_, bool inMainLoop_) : 
        id(id_gen++), level(level_), inMainLoop(inMainLoop_)
    {}
    
    void resetId() {
        id_gen = 0;
    }

    void setLevel(unsigned level_) {
        level = level_;
    }
    unsigned getLevel() {
        return level;
    }
    unsigned getCorrLevel() {
        return ((inMainLoop && level != 0) ? level-1 : level);
    }
    bool isMainLoop() {
        return inMainLoop;
    }
    bool isDead() {
        return (id == 0);
    }
    
    std::string asString() {
        return ("S"+std::to_string(id));
    }
    
    bool operator == (const CodeScope& rhs) const {
        return (rhs.id == id);
    }
};

}

namespace sc {
    
typedef std::pair<std::shared_ptr<CodeScope>, 
                  std::vector<std::shared_ptr<CodeScope>> > ScopeRelation;

/// Analyzed scope vector, prepared for their successors
/// Sorted by scope level, started with minimal level
class PreparedScopes : public std::list<ScopeRelation> 
{
public:
    
    explicit PreparedScopes() : 
        std::list<ScopeRelation>()
    {}
    
    void addScope(const ScopeRelation& scopeRel)
    {
        auto i = rbegin();
        for (; i != rend(); ++i) {
            if (i->first == scopeRel.first) {
                i->second.insert(i->second.end(), scopeRel.second.begin(), 
                                 scopeRel.second.end());
                break;
            }
        }
        if (i == rend()) {
            // Insert scope according with its level
            unsigned succLevel = scopeRel.first->getCorrLevel();
            auto j = begin();
            for (; j != end(); ++j) {
                if (succLevel < j->first->getCorrLevel()) {
                    break;
                }
            }
            // Insert new scope before element pointed by @j
            insert(j, scopeRel);
        }            
    }
    
    void splice(const PreparedScopes& other)
    {
        for (const auto& i : other) {
            addScope(i);
        }
    }
};
}

namespace std {

/// Hash function for @SVariable
template<> 
struct hash< std::pair<const clang::Stmt*, bool> >  
{
    std::size_t operator () (const std::pair<const clang::Stmt*, bool>& obj) const;
};

}

namespace sc {
/// ...
class ScScopeGraph {
public:
    ScScopeGraph() : 
        lastFuncCall(nullptr),
        firstScope(nullptr),
        currScope(nullptr)
    {}
    
    virtual ~ScScopeGraph() {
    }
    
    //=========================================================================

    /// Store given string for general statement in the current scope
    void storeStmt(const clang::Stmt* stmt, const std::string& s,  
                   bool artifIf = false);
    
    // Add comment for statement
    void addComment(const clang::Stmt* stmt, const std::string& comment);
    
    /// Store @sct_assert statement in the current scope
    void storeAssertStmt(const clang::Stmt* stmt, const std::string& s);

    /// Store null statement to make scope non-empty, required for do..while
    void storeNullStmt(std::shared_ptr<CodeScope> scope);
    
    /// Set statement as belong to METHOD with empty sensitive, 
    /// no tabulation used in print this statement
    void setEmptySensStmt(const clang::Stmt* stmt);

    /// Store switch case value expression
    /// \param empty -- case is empty (no statements and no break)
    void storeCase(std::shared_ptr<CodeScope> scope, 
                   const std::string& s, bool empty);
    
    /// Store THREAD state variable assignment next state value
    void storeStateAssign(const clang::Stmt* stmt, 
                          const std::pair<std::string,std::string>& names, 
                          size_t waitId, bool isReset, bool addTab = false,
                          const std::string& comment = "");

public:
    /// Set/get current scope
    void setFirstScope(std::shared_ptr<CodeScope> scope);
    std::shared_ptr<CodeScope> getFirstScope();
    
    /// Set/get current scope
    void setCurrScope(std::shared_ptr<CodeScope> scope);
    std::shared_ptr<CodeScope> getCurrScope();

    /// Set/get current block level as maximal of predecessors
    void setCurrLevel(unsigned level);
    /// Get original level as used in TraverseProc, no correction by InMainLoop
    unsigned getCurrLevel();
    
    /// Set level for the given scope
    void setScopeLevel(std::shared_ptr<CodeScope> scope, unsigned level);

    /// Add @succ to successors of @currScope
    void addScopeSucc(std::shared_ptr<CodeScope> succ);

    /// Add @succ to successors of @scope
    void addScopeSucc(std::shared_ptr<CodeScope> scope, 
                      std::shared_ptr<CodeScope> succ);

    /// Add @scope to @currScope predecessors
    void addScopePred(std::shared_ptr<CodeScope> pred);
    
    /// Add @scope to @currScope predecessors
    void addScopePred(std::shared_ptr<CodeScope> scope, 
                      std::shared_ptr<CodeScope> pred);
    
    //=========================================================================

    /// Get tabulation string with length corresponds scope depth
    /// \corr -- tab number correction, can be negative
    std::string getTabString(std::shared_ptr<CodeScope> scope, int corr = 0);
    std::string getTabString(unsigned level);
    
    /// Set function name
    /// \param funcCall -- real function call, not break/continue
    void setName(const std::string& fname, bool fcall = true);

    /// Add scope graph for function call
    void addFCallScope(const clang::Stmt* stmt, const clang::Stmt* loopTerm,
                       std::shared_ptr<ScScopeGraph> graph);
    
    void putVarAssignStmts(const std::unordered_map<SValue, std::unordered_set<
                           const clang::Stmt*>>& stmts) 
    {
        using std::cout; using std::endl;
        stmtAssignVars.clear();
        //cout << "putVarAssignStmts --------" << endl; 
        for (auto i : stmts) {
            for (auto stmt : i.second) {
                //cout << "stmt " << std::hex << stmt << " val " << i.first << endl;
                stmtAssignVars.emplace(stmt, i.first);
            }
        }
    }
    
    /// Print scope statements and included scopes
    /// \param printLevel -- level for output tabs
    /// \param noExitByLevel -- no return from this printCurrentScope() by level up 
    PreparedScopes printCurrentScope(std::ostream &os, 
                                     std::shared_ptr<CodeScope> scope, 
                                     unsigned printLevel, 
                                     bool noExitByLevel = false);
    /// Clear @visited after print to prepare to next print
    void clearAfterPrint();

    /// Print all scopes
    void printAllScopes(std::ostream &os);
    
    /// Clone scope graph at @wait()
    /// \param innerGraph -- inner graph for last function call or nullptr
    std::unique_ptr<ScScopeGraph> clone(std::shared_ptr<ScScopeGraph> innerGraph,
                                        bool inMainLoop);
    
protected:
    /// Assignment symbol "=" or "<="
    std::string ASSIGN_SYM = " = ";
    std::string NB_ASSIGN_SYM = " <= ";
    /// Begin and end scope symbols
    std::string BEGIN_SYM = "begin";
    std::string END_SYM = "end";
    std::string ENDCASE_SYM = "endcase";
    /// Tabulation
    std::string TAB_SYM = "    ";
    /// THREAD state variable name
    std::string STATE_VAR_NAME = "PROC_STATE";

    /// Number of recursively passed scope predecessors to remove loop 
    /// sub-statements with @removeStmt()
    const unsigned REMOVE_STMT_PRED = 5;
    /// Maximal allowed level, restricted to detect level overflow error
    unsigned MAX_LEVEL  = 100;
            
    /// Scope graph name and function call flag
    std::string name;
    bool funcCall;
    /// Visited scopes
    std::unordered_set<std::shared_ptr<CodeScope>>   visited;
    // Scope to scope level reflection
    std::unordered_map<std::shared_ptr<CodeScope>, unsigned>  scopeLevel;
    // Scope successors
    std::unordered_map<std::shared_ptr<CodeScope>, 
                       std::vector<std::shared_ptr<CodeScope>> >  scopeSuccs;
    // Scope predecessors
    std::unordered_map<std::shared_ptr<CodeScope>, 
                       std::vector<std::shared_ptr<CodeScope>> >  scopePreds;

    /// Switch values for case scopes
    /// <scope, <case string, empty case>>
    std::unordered_map<std::shared_ptr<CodeScope>, 
                       std::pair<std::string, bool>> switchCases;
    /// Function call, break/continue included ScopeGraph`s
    /// <statement, wait flag>, wait flag is true in context at wait() call
    /// <loop statement, scope graph>, loop statement is used to avoid removing
    ///                                function call in loop condition/initialization 
    std::unordered_map<std::pair<const clang::Stmt*, bool>, 
                       std::pair<const clang::Stmt*, 
                                 std::shared_ptr<ScScopeGraph> >> innerScopeGraphs;
    /// Last call statement, used to clone scope graph in wait() 
    const clang::Stmt* lastFuncCall;
    
    /// First scope
    std::shared_ptr<CodeScope>  firstScope;
    /// Current scope
    std::shared_ptr<CodeScope>  currScope;
    
    /// Loop statements which are artificial IF in threads
    std::unordered_set<const clang::Stmt*>  artifIfStmts;
    /// @sct_assert statements
    std::unordered_set<const clang::Stmt*>  assertStmts;
    
    /// Statements belong to METHOD with empty sensitive
    std::unordered_set<const clang::Stmt*>  emptySensStmt;
    /// Comments for statements
    std::unordered_map<const clang::Stmt*, std::string> stmtComments;
    
    /// Variables and constants not replaced by integer values
    static std::unordered_set<SValue> notReplacedVars;
    /// Statement assigned variable, used to remove variable initialization 
    /// statements for removed variables/constants
    static std::unordered_map<const clang::Stmt*, SValue> stmtAssignVars;
};

}

#endif /* SCSCOPEGRAPH_H */

