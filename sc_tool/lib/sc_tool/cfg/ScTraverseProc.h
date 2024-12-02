/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SC Clang front end project.
 * 
 * SC process function traverse class.
 *  
 * File:   ScTraverseProc.h
 * Author: Mikhail Moiseev
 */

#ifndef SCTRAVERSEPROC_H
#define SCTRAVERSEPROC_H

#include "sc_tool/scope/ScVerilogWriter.h"
#include "sc_tool/expr/ScGenerateExpr.h"
#include "sc_tool/scope/ScScopeGraph.h"
#include <sc_tool/cthread/ScCThreadStates.h>
#include <sc_tool/cthread/ScFindWaitCallVisitor.h>
#include "sc_tool/cthread/ScCfgCursor.h"
#include "sc_tool/cfg/ScTraverseCommon.h"
#include "sc_tool/cfg/ScStmtInfo.h"
#include "sc_tool/utils/CfgFabric.h"
#include "clang/Analysis/CFG.h"

#include <climits>
#include <memory>

namespace sc {
    
/// Loop stack type 
struct LoopStackInfo {
    // Loop statement
    const clang::Stmt* stmt;
    // Scope level
    unsigned level;
    // Loop has no body or removed by CTHREAD loop transformation
    bool removed;
    
    bool operator ==(const LoopStackInfo& other) {
        return (stmt == other.stmt);
    }
    
    bool operator ==(const LoopStackInfo& other) const {
        return (stmt == other.stmt);
    }
    
    LoopStackInfo(const clang::Stmt* stmt_, unsigned level_, bool removed_) :
        stmt(stmt_), level(level_), removed(removed_)
    {}
};

class LoopStack : public std::vector<LoopStackInfo> 
{
public:
    // Push loop into stack without duplicating
    void pushLoop(const LoopStackInfo& info) 
    {
        using namespace std;
        //cout << "Push loop " << hex << info.stmt << " at level "  << dec << info.level << endl;
        
        if ( !isCurrLoop(info.stmt) ) {
            // Check this loop is not previous loops
            if (hasLoopStmt(info.stmt)) {
                print();
                cout << " New loop pushed " << hex << info.stmt << dec << endl;
                SCT_TOOL_ASSERT (false, "Duplicate loop in loop stack, "
                                 "maybe miss break after switch case");
            }
            
            push_back(info);
        }
    }
    
    // Check if last loop equal to given one and pop it
    void popLoop(const clang::Stmt* stmt) 
    {
        if ( isCurrLoop(stmt) ) {   
            pop_back();
        }
    }
    
    // Check if there is given loop statement in the loop stack
    bool hasLoopStmt(const clang::Stmt* stmt) const {
        for (const auto& i : *this) {
            if (i.stmt == stmt) {
                return true;
            }
        }
        return false;
    }
    
    // Check if given statement is current loop statement and 
    // it is not removed loop
    bool isCurrLoop(const clang::Stmt* stmt) const 
    {
        if (empty()) {
            return false;
        }
        return (back().stmt == stmt);
    }
    
    // Print loop stack
    void print() const {
        using namespace std;
        
        cout << "Loop stack : " << endl;
        for (auto& i : *this) {
            
            cout << "  " << hex << i.stmt << dec 
                 << (llvm::isa<const clang::SwitchStmt>(i.stmt) ? " S" : "") << endl;
        }
        cout << "----------------------------" << endl;
    }
};

/// Predecessor @scope, @state and @level values to store in @delayes
struct ScopeInfo 
{
    std::shared_ptr<CodeScope>  prevScope;
    std::shared_ptr<CodeScope>  currScope;
    
    /// Loop statement stack, last loop is most inner
    LoopStack loopStack;
    
    /// Dead predecessor should be ignored in preparing next block
    /// Used for &&/|| dead branch leads final IF
    bool deadCond;
    
    ScopeInfo(std::shared_ptr<CodeScope>  prevScope_,
              std::shared_ptr<CodeScope>  currScope_,
              const LoopStack& loopStack_,
              bool deadCond_ = false
              ) : 
        prevScope(prevScope_),
        currScope(currScope_),
        loopStack(loopStack_),
        deadCond(deadCond_)
    {}
};


/// Function call context, used to restore context in return from function call
struct ScFuncContext {
    /// CFG block and element
    CfgCursor callPoint;
    /// Return value, temporary variable declared in parent function
    SValue returnValue;
    /// Current module value, *this if it is not in local record
    SValue modval;
    /// Current local record value, *this
    SValue recval;
    /// Delayed Clang CFG blocks with extended scope predecessors 
    std::list< std::pair<AdjBlock, std::vector<ScopeInfo> > >  delayed;
    // Loop statement stack, last loop is most inner
    LoopStack loopStack;
    /// Function directly called from this function 
    /// <func expr, <return value, return pointed object>>
    std::unordered_map<clang::Stmt*, std::pair<SValue, SValue>>  calledFuncs;
    /// Context stored in break/continue in removed loop analysis run,
    /// do not skip current block element as for function call
    bool breakContext;
    /// Current function has all the branches stopped at wait() calls, no exit achieved
    bool noExitFunc;
    
    /// Scope graph printer
    std::shared_ptr<ScScopeGraph> scopeGraph;
    
    /// State sensitive part of @ScVerilogWriter
    ScVerilogWriterContext  codeWriter;
    
    explicit ScFuncContext(
                    const CfgCursor& callPoint_,
                    const SValue& returnValue_,
                    const SValue& modval_,
                    const SValue& recval_,
                    const std::list< std::pair<AdjBlock, 
                          std::vector<ScopeInfo> > >& delayed_,
                    const LoopStack& loopStack_,
                    const std::unordered_map<clang::Stmt*, 
                          std::pair<SValue, SValue>>& calledFuncs_,
                    bool breakContext_, bool noExitFunc_,
                    const std::shared_ptr<ScScopeGraph>& scopeGraph_,
                    const ScVerilogWriterContext& codeWriter_) :
            callPoint(callPoint_), returnValue(returnValue_), 
            modval(modval_), recval(recval_),
            delayed(delayed_), loopStack(loopStack_), 
            calledFuncs(calledFuncs_), 
            breakContext(breakContext_), noExitFunc(noExitFunc_),
            scopeGraph(scopeGraph_), codeWriter(codeWriter_)
    {}
    
    /// Create copy of @this with allocating new memory to scope graph pointer
    ScFuncContext clone(std::shared_ptr<ScScopeGraph> innerGraph, bool inMainLoop) 
    {
        ScFuncContext res(*this);
        res.scopeGraph = scopeGraph->clone(innerGraph, inMainLoop);
        return res;
    }
};

/// Process context which describes a point in function call stack with contexts
class ScProcContext : public std::vector<ScFuncContext>
{
public:
    CfgCursorStack getCursorStack() 
    {
        CfgCursorStack cursorStack;

        for (auto& ctx : *this) {
            cursorStack.push_back(ctx.callPoint);
        }        
        return cursorStack;
    }
    
    CallStmtStack getStmtStack()
    {
        using namespace clang;
        CallStmtStack stmtStack;

        for (auto& ctx : *this) {
            auto& cursor = ctx.callPoint;
            auto elm = cursor.getBlock()->operator [](cursor.getElementID());
            
            if (elm.getKind() == CFGElement::Kind::Statement) {
                CFGStmt cfgstmt = *elm.getAs<CFGStmt>();
                stmtStack.push_back(cfgstmt.getStmt());
                
            } else {
                SCT_TOOL_ASSERT (false, "Incorrect element kind");
            }
        }        
        return stmtStack;
    }
    
    void printCursorStack() 
    {
        using namespace std;
        cout << "Cursor stack ("<< size() << "): " << endl;
        for (auto& ctx : *this) {
            cout << "    " << ctx.callPoint.getFuncDecl()->getNameAsString() << endl;
        }        
        cout << endl;
    }
    
    /// Create copy of @this with allocating new memory to scope graph pointer
    /// in each function context
    ScProcContext clone(bool inMainLoop) 
    {
        ScProcContext res;
        
        // Last graph has no inner graph
        std::shared_ptr<ScScopeGraph> innerGraph = nullptr;
        for (auto i = rbegin(); i != rend(); ++i) {
            auto funcCtx = i->clone(innerGraph, inMainLoop);
            res.push_back(funcCtx);
            innerGraph = funcCtx.scopeGraph;
        } 

        // Reverse order
        std::reverse(res.begin(), res.end());
        
        return res;
    }
};

/// Process function analyzer, @state is cloned and owned by ScTraverseProc 
class ScTraverseProc : public ScGenerateExpr {
public:
    /// @state cloned in @ScGenerateExpr and released in its destructor
    explicit ScTraverseProc(const clang::ASTContext& context_, 
                            std::shared_ptr<ScState> state_, 
                            const SValue& modval_,
                            ScVerilogWriter* codeWriter_,
                            const ScCThreadStates* cthreadStates_ = nullptr,
                            const FindWaitCallVisitor* findWaitInLoop_ = nullptr,
                            bool isCombProcess_ = false,
                            bool isSingleStateThread = false) :
        ScGenerateExpr(context_, state_, isCombProcess_, modval_, codeWriter_),
        cthreadStates(cthreadStates_),
        findWaitInLoop(findWaitInLoop_),
        isSingleStateThread(isSingleStateThread)
    {}

    virtual ~ScTraverseProc() {
    }
    
protected:
    static std::shared_ptr<CodeScope> deadScope;
    
    /// Store statement string
    void storeStmtStr(clang::Stmt* stmt);
    
    /// Store statement string for @nullptr
    void storeStmtStrNull(clang::Stmt* stmt);
    
    /// Create new code scope with specified level
    std::shared_ptr<CodeScope> createScope(unsigned level);
    
    /// Get code scope for block or create new one
    std::shared_ptr<CodeScope> getScopeForBlock(AdjBlock block, unsigned level);
    
    /// Generate temporal assertion inside of loop(s) if required
    std::optional<std::string> getSvaInLoopStr(const std::string& svaStr, 
                                                bool isResetSection);
    
    /// Check if function has wait() inside from CPA stored in @hasWaitFuncs
    bool isWaitInFunc(const clang::FunctionDecl* decl) override;
    
    /// Check if current function has wait() inside
    bool isWaitInCurrFunc() override;
    
    /// Get terminator condition from CPA stored in @termConds
    /// \param val  -- at all iterations including first
    /// \param fval -- at first iteration only, used for loop with wait()
    void getTermCondValue(const clang::Stmt* stmt, SValue& val, 
                          SValue& fval) override;

    /// Check then/else branches are empty
    bool checkIfBranchEmpty(const clang::Stmt* branch);
    
    /// Prepare next block analysis
    void prepareNextBlock(AdjBlock& nextBlock, std::vector<ScopeInfo>& scopeInfos);
    
    void putWaitScopeGraph(const clang::Stmt* stmt, int waitId, bool isResetSection);

    /// Put counter check and decrease code for wait(n) state entrance
    void putWaitNScopeGraph(const clang::Stmt* stmt, int waitId, 
                            bool isResetSection);

    /// Register return value and prepare @lastContext, 
    /// used for methods and global functions
    void prepareCallContext(clang::Expr* expr, const SValue& funcModval, 
                            const SValue& funcRecval, 
                            const clang::FunctionDecl* callFuncDecl, 
                            const SValue& retVal) override;
    
    /// Function call expression
    void parseCall(clang::CallExpr* expr, SValue& val) override;
    
    /// Member function call expression
    void parseMemberCall(clang::CXXMemberCallExpr* expr, SValue& tval, 
                         SValue& val) override;
    
    /// Operator call expression
    void parseOperatorCall(clang::CXXOperatorCallExpr* expr, SValue& tval,
                           SValue& val) override;
    
    /// Choose and run DFS step in accordance with expression type.
    /// Remove sub-statements from generator
    void chooseExprMethod(clang::Stmt *stmt, SValue &val) override;
    
    /// Initialize analysis context at function entry
    void initContext();
    
    /// Restore analysis context with given one
    /// \param funcCallRestore -- restore scope graph and delayed after function call,
    ///                           not restored for start analysis in wait()                       
    /// \return -- is context stored in break/continue in removed loop analysis run
    bool restoreContext(bool funcCallRestore);
    
public:
    /// Preset CFG for run analysis
    void setFunction(clang::FunctionDecl* fdecl);
    
    /// Set empty sensitivity for method process
    void setEmptySensitivity(bool empty);
    
    /// Set context stack, used for thread wait-to-wait analysis
    void setContextStack(const ScProcContext& context);
    
    /// Run analysis at function entry or at wait() call
    void run();
    
    /// Run for function declaration, the same as @setFunction() and @run()
    void run(clang::FunctionDecl* fdecl, bool emptySensitivity);
    
    /// Run for context stack, the same as @setContextStack() and @run()
    void run(const ScProcContext& context);

    /// Run for SVA property declaration, result string returned in success
    std::optional<std::string> runSvaDecl(const clang::FieldDecl* fdecl);
    
    /// Print function local variable declarations
    void printLocalDeclarations(std::ostream &os);

    /// Print function local combinational variable declarations (for reset section)
    void printResetDeclarations(std::ostream &os);

    /// Print in-reset initialization for local variables which become registers
    /// declared in CTHREAD main loop body (no reset for them)
    void printInitLocalInReset(std::ostream &os);

    /// Print function statements
    void printFunctionBody(std::ostream &os);
    
    /// Print temporal assertion in thread process, printed in @always_ff
    /// \param forReset -- for reset section if true, others otherwise
    void printTemporalAsserts(std::ostream &os, bool forReset);
    
    /// Get wait contexts
    std::vector<ScProcContext>& getWaitContexts();
    
    void setLiveStmts(const std::unordered_set<clang::Stmt*>& stmts) {
        liveStmts = stmts;
    }
    
    void setLiveTerms(const std::unordered_set<clang::Stmt*>& stmts) {
        liveTerms = stmts;
    }
    
    void setRemovedArgExprs(const std::unordered_set<clang::Stmt*>& stmts) {
        removedArgExprs = stmts;
    }
    
    /// Get evaluated terminator condition values
    void setTermConds(const std::unordered_map<CallStmtStack, SValue>& conds);
    
    /// Filter and set functions evaluated as constants
    void setConstEvalFuncs(const std::unordered_map<CallStmtStack, SValue>& funcs);
    
    /// Current process has reset signal
    void setHasReset(bool hasReset_) {
        hasReset = hasReset_;
    }
    
    bool getHasReset() {
        return hasReset;
    }
    
    /// Set functions with wait()
    void setWaitFuncs(const std::unordered_set<const clang::FunctionDecl*>& funcs) {
        hasWaitFuncs = funcs;
    }
    
    void setMainLoopStmt(const clang::Stmt* stmt) {
        mainLoopStmt = stmt;
    }
    
    void setReplacedStates(const std::unordered_map<WaitID, WaitID>& states) {
        replacedStates = states;
    }
    
    /// Report error for lack/extra sensitive to SS channels
    void reportSctChannel(sc_elab::ProcessView procView,
                          const clang::FunctionDecl* funcDecl);
    
protected:
    /// CFG fabric singleton
    CfgFabric* cfgFabric = CfgFabric::getFabric(astCtx);
    /// Level, sub-statement and other statement information provider
    ScStmtInfo stmtInfo;
    
    /// Current CFG block in analysis
    AdjBlock block;
    /// Element index in CFG block
    size_t elemIndx; 
    /// Exit from function CFG block ID
    unsigned exitBlockId;
    
    /// ID of wait statement, used as process state value
    WaitID waitId;
    /// Duplicated states replaced with another, <original state, replacement state>
    std::unordered_map<WaitID, WaitID> replacedStates;

    /// Current function 
    const clang::FunctionDecl* funcDecl = nullptr;
    /// Current function CFG
    clang::CFG* cfg;
    /// Scope graph printer
    std::shared_ptr<ScScopeGraph> scopeGraph = nullptr;
    /// Delayed Clang CFG blocks with extended scope predecessors 
    std::list< std::pair<AdjBlock, std::vector<ScopeInfo> > >  delayed;
    
    /// Loop statement stack, last loop is most inner
    LoopStack loopStack;
    /// Function directly called from this function 
    /// <func expr, <return value, return pointed object>>
    std::unordered_map<clang::Stmt*, std::pair<SValue, SValue>>  calledFuncs;
    // Function call statement and string in loop condition
    // <loop stmt, vector<call expr, call string>>
    std::map<clang::Stmt*, std::vector<std::pair<
                           clang::Stmt*, std::string>>> condFuncCallLoops;
    
    /// Temporal assert statements for reset section and all others
    InsertionOrderSet<std::string> sctRstAsserts;
    InsertionOrderSet<std::string> sctAsserts;
    
    // Dead condition on path from &&/|| terminator to final IF/Loop/? condition, 
    // used to avoid dead condition generation
    bool deadCond = false;
    /// Wait call in last statement
    bool noExitFunc = false;

    /// Wait call in last statement
    bool waitCall = false;
    /// Function call in last statement
    bool funcCall = false;
    /// Context for last called function
    std::shared_ptr<ScFuncContext> lastContext = nullptr;
    /// Call context stack
    ScProcContext  contextStack;
    /// Contexts stored for break/continue separate analysis
    std::vector<ScProcContext> storedContextStacks;
    /// Contexts stored in wait() calls, returned to CTHREAD analyzer
    std::vector<ScProcContext>  waitContexts;
    /// Level for current function entry, added with block level in function
    //unsigned funcLevel = 0;

    /// Live statements from ScTraverseConst
    std::unordered_set<clang::Stmt*> liveStmts;    
    /// Live terminators, also includes switch cases/default
    std::unordered_set<clang::Stmt*> liveTerms;
    /// Evaluated terminator condition values, used in ScTraverseProc
    /// To distinguish FOR/WHILE first iteration loop terminator is 
    /// placed into #CallStmtStack twice, and once for other iterations
    std::unordered_map<CallStmtStack, SValue> termConds;
    /// Functions evaluated as constants if stored SValue is integer or 
    /// not eligible if NO_VALUE stored
    std::unordered_map<CallStmtStack, SValue> constEvalFuncs;
    
    /// THREAD wait states and constant propagation result providers
    const ScCThreadStates* cthreadStates = nullptr;
    const FindWaitCallVisitor* findWaitInLoop = nullptr;

    /// Method with NO sensitivity list, @assign statement generated for such method
    bool emptySensitivity = false;
    /// Current process has reset signal
    bool hasReset;
    /// Main loop terminator for CTHREAD
    const clang::Stmt* mainLoopStmt = nullptr;
    /// PROC_STATE is not generated for threads with only a 1 state
    bool isSingleStateThread;
    /// Entered into main loop of CTHREAD process
    bool inMainLoop = false;
    
    /// Functions with wait()
    std::unordered_set<const clang::FunctionDecl*>  hasWaitFuncs;

 public:    
    /// SS channels used in the process, should be in sensitivity list
    std::unordered_set<sc_elab::ObjectView> usedSctChannels;
    /// SS targets with FIFO inside, should be skipped as FIFO used in sensitivity 
    std::unordered_set<sc_elab::ObjectView> skipSctTargets;

    /// Normal statements, terminators, assertions, wait calls
    std::unordered_set<const clang::Stmt*> statStmts;
    std::unordered_set<const clang::Stmt*> statTerms;
    std::unordered_set<const clang::Stmt*> statAsrts;
    std::unordered_set<const clang::Stmt*> statWaits;
    
};

}


#endif /* SCTRAVERSEPROC_H */

