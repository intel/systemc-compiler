/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SC Clang front end project.
 * 
 * SC process function traverse class for constant propagation analysis.
 *  
 * File:   ScTraverseConst.h
 * Author: Mikhail Moiseev
 */

#ifndef SCTRAVERSECONST_H
#define SCTRAVERSECONST_H

#include "sc_tool/cfg/ScStmtInfo.h"
#include "sc_tool/expr/ScParseExprValue.h"
#include "sc_tool/scope/ScScopeGraph.h"
#include "sc_tool/cthread/ScCThreadStates.h"
#include "sc_tool/cthread/ScFindWaitCallVisitor.h"
#include "sc_tool/cthread/ScCfgCursor.h"
#include "sc_tool/cfg/ScTraverseCommon.h"
#include "sc_tool/utils/CfgFabric.h"
#include "clang/Analysis/CFG.h"

#include <climits>
#include <memory>

namespace sc_elab {
    class ElabDatabase;
}

namespace sc {

/// Loop stack type 
struct ConstLoopStackInfo {
    // Loop statement
    const clang::Stmt* stmt;
    // Last iteration state
    std::shared_ptr<ScState> state;
    // Iteration number or 0 if unknown, 
    // do not need to consider zero iteration loops
    unsigned iterNumber;
    // Iteration counter, used in constant propagator
    unsigned char counter;
    // Last state comparison results in stable state
    bool stableState;
   
    bool operator ==(const ConstLoopStackInfo& other) {
        return (stmt == other.stmt);
    }
    
    bool operator ==(const ConstLoopStackInfo& other) const {
        return (stmt == other.stmt);
    }
    
    ConstLoopStackInfo() = delete;
    
    ConstLoopStackInfo(const clang::Stmt* stmt_, ScState* state_, 
                       unsigned iterNumber_, unsigned counter_) :
        stmt(stmt_), state(state_), 
        iterNumber(iterNumber_), counter(counter_), stableState(false)
    {}
};

class ConstLoopStack : public std::vector<ConstLoopStackInfo> 
{
public:
    // Push loop into stack without duplicating
    void pushLoop(const ConstLoopStackInfo& info) 
    {
        using namespace std;
        
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

/// Predecessor @scope, @state and others to store in @delayes
struct ConstScopeInfo 
{
    std::shared_ptr<ScState> state;
    /// Previous block
    AdjBlock prevBlock;
    
    /// Loop statement stack, last loop is most inner
    ConstLoopStack loopStack;
    
    /// Visited loops, used to detect loop with wait() having combinational path
    std::unordered_set<const clang::Stmt*>   visitedLoops;
    
    /// Dead predecessor should be ignored in preparing next block
    /// Used for &&/|| dead branch leads final IF
    bool deadCond;
    
    ConstScopeInfo(std::shared_ptr<ScState> state_,
              const AdjBlock& prevBlock_,
              const ConstLoopStack& loopStack_,
              const std::unordered_set<const clang::Stmt*>&  visitedLoops_,
              bool deadCond_ = false
              ) : 
        state(state_),
        prevBlock(prevBlock_),
        loopStack(loopStack_),
        visitedLoops(visitedLoops_),
        deadCond(deadCond_)
    {}
};

/// Function call context, used to restore context in return from function call
struct ConstFuncContext {
    /// CFG block and element
    CfgCursor callPoint;
    /// Return value, temporary variable declared in parent function
    SValue returnValue;
    /// Current module value, *this
    SValue modval;
    /// Current local record value, *this
    SValue recval;
    /// Delayed Clang CFG blocks with extended scope predecessors 
    std::list< std::pair<AdjBlock, std::vector<ConstScopeInfo> > >  delayed;
    // Loop statement stack, last loop is most inner
    ConstLoopStack loopStack;
    /// Function directly called from this function <func expr, return value>
    std::unordered_map<clang::Stmt*, SValue>  calledFuncs;
    /// Current function has one return from function scope and return statement
    bool simpleReturnFunc;
    clang::Stmt* returnStmtFunc;
    /// Current function and all called functions change some non-local
    /// variables/channels through parameters or directly
    bool sideEffectFunc;
   
    explicit ConstFuncContext(
                    const CfgCursor& callPoint_,
                    const SValue& returnValue_,
                    const SValue& modval_,
                    const SValue& recval_,
                    const std::list< std::pair<AdjBlock, 
                          std::vector<ConstScopeInfo> > >& delayed_,
                    const ConstLoopStack& loopStack_,
                    const std::unordered_map<clang::Stmt*, SValue>& calledFuncs_,
                    bool simpleReturnFunc_,
                    clang::Stmt* returnStmtFunc_,
                    bool sideEffectFunc_
                    ) :
            callPoint(callPoint_), returnValue(returnValue_), 
            modval(modval_), recval(recval_),
            delayed(delayed_), loopStack(loopStack_), 
            calledFuncs(calledFuncs_), 
            simpleReturnFunc(simpleReturnFunc_), returnStmtFunc(returnStmtFunc_),
            sideEffectFunc(sideEffectFunc_)
    {}
};

/// Process context which describes a point in function call stack with contexts
class ConstProcContext : public std::vector<ConstFuncContext>
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
};

/// Process function analyzer, @state is cloned and owned by ScTraverseConst 
class ScTraverseConst : public ScParseExprValue {
public:
    /// @state cloned in @ScGenerateExpr and released in its destructor
    explicit ScTraverseConst(const clang::ASTContext& context_, 
                             std::shared_ptr<ScState> state_, 
                             const SValue& modval_,
                             std::shared_ptr<ScState> globalState_ = nullptr,
                             sc_elab::ElabDatabase *elabDB = nullptr,
                             ScCThreadStates* cthreadStates_ = nullptr,
                             const FindWaitCallVisitor* findWaitInLoop_ = nullptr,
                             bool isCombProcess = false,
                             bool isMethProcess = false) :
        ScParseExprValue(context_, state_, isCombProcess,  modval_, false),
        cthreadStates(cthreadStates_), 
        findWaitInLoop(findWaitInLoop_),
        globalState(globalState_), 
        globExprEval(astCtx, globalState, true, modval),
        elabDB(elabDB), 
        dynmodval(modval_),
        isSeqMethod(!isCombProcess && isMethProcess)
    {
        state->getMostDerivedClass(dynmodval, dynmodval);
    }

    /// @state deleted in @ScTraverse destructor
    virtual ~ScTraverseConst() {
    }
    
protected:
    /// Creates constant value in global state if necessary
    /// Used for constant and constant array belong to template parameter class
    void parseGlobalConstant(const SValue& val) override;

    /// Register variables accessed in and after reset section,
    /// check read-not-defined is empty in reset
    void registerAccessVar(bool isResetSection, const clang::Stmt* stmt);
    
    /// Evaluate terminator condition if it is compile time constant
    void evaluateTermCond(clang::Stmt* stmt, SValue& val);
    
    /// Evaluate literal or constant expression as non-negative integer
    std::optional<unsigned> evaluateConstExpr(clang::Expr* expr);

    /// Evaluate loop iteration number from conditional expression
    std::optional<unsigned> evaluateIterNumber(const clang::Stmt* stmt);
    
    /// Store ternary statement condition for SVA property
    void putSvaCondTerm(const clang::Stmt* stmt, SValue val) override;
    
    /// Prepare next block analysis
    void prepareNextBlock(AdjBlock& nextBlock, std::vector<ConstScopeInfo>& scopeInfos);
    
    /// Register return value and prepare @lastContext, 
    /// used for methods and global functions
    void prepareCallContext(clang::Expr* expr, const SValue& funcModval, 
                            const SValue& funcRecval, 
                            const clang::FunctionDecl* callFuncDecl, 
                            const SValue& retVal) override;
    
    /// Parse and return integer value of wait()/wait(N) argument
    unsigned parseWaitArg(clang::CallExpr* expr);
    
    /// Return statement
    void parseReturnStmt(clang::ReturnStmt* stmt, SValue& val) override;

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
    void restoreContext();
    
public:
    /// Remove unused variable definition statements in METHODs and CTHREADs
    void removeUnusedStmt();
    
    /// Run analysis at function entry, runs once per process
    void run();
    
    /// Run for function declaration, the same as @setFunction() and @run()
    void run(sc_elab::VerilogModule* verMod, const clang::FunctionDecl* fdecl);
    
    ///
    const std::unordered_set<clang::Stmt*>& getLiveStmts() const {
        return liveStmts;        
    }
    
    const std::unordered_set<clang::Stmt*>& getLiveTerms() const {
        return liveTerms;        
    }
    
    const std::unordered_set<clang::Stmt*>& getRemovedArgExprs() const {
        return removedArgExprs; 
    }
    
    /// Get evaluated terminator condition values
    const std::unordered_map<CallStmtStack, SValue>& getTermConds() const {
        return termConds;        
    }
   
    /// Get values defined in reset section
    const std::unordered_set<SValue>& getResetDefConsts() const {
        return resetDefinedConsts;
    }
    
    /// Check if given variable value accessed in CTHREAD reset
    bool isInResetAccessed(const SValue& val) const {
        return (inResetAccessVars.count(val) != 0);
    }
    
    /// Check if given variable value accessed after CTHREAD reset
    bool isAfterResetAccessed(const SValue& val) const {
        return (afterResetAccessVars.count(val) != 0);
    }

    /// Check if given variable value accessed in process SVA
    bool isSvaProcAccessed(const SValue& val) const {
        return (inSvaAccessVars.count(val) != 0);
    }
    
    /// Get functions with wait()
    const std::unordered_set<const clang::FunctionDecl*>& getWaitFuncs() const {
        return hasWaitFuncs;
    }
    
    /// Set maximal iteration number analyzed for a loop
    void setLoopMaxIter(unsigned value) {
        LOOP_MAX_ITER = value;
    }
    
    /// Current thread has reset signal
    void setHasReset(bool hasReset_);
    
    /// Get stored state at wait() calls
    std::map<WaitID, ScState>& getWaitStates() {
        return waitStates;
    }
    
    /// Get wait states ordered, really that just numbers from 0 to N-1
    std::vector<WaitID> getWaitStatesInOrder() {
        std::vector<WaitID> res;
        for (auto& entry : waitStates) {
            res.push_back(entry.first);
        }
        return res;
    }
    
    /// Get final state with UseDef information
    ScState* getFinalState() const {
        return finalState.get();
    }
    
    /// Get defined/used values after unused statements removed
    std::unordered_set<SValue> getDefinedVals() {
        std::unordered_set<SValue> definedVals;
        for (const auto& i : defVarStmts) {
            definedVals.insert(i.first);
        }
        return definedVals;
    }
    
    std::unordered_set<SValue> getUsedVals() {
        std::unordered_set<SValue> usedVals;
        for (const auto& i : useVarStmts) {
            usedVals.insert(i.first);
        }
        return usedVals;
    }
    
    std::unordered_map<CallStmtStack, SValue> getConstEvalFuncs() {
        return constEvalFuncs;
    }
    
    /// Get main loop terminator for CTHREAD
    const clang::Stmt* getMainLoopStmt() const {
        return mainLoopStmt;
    }
    
    /// Check if CTHREAD has code before main loop
    bool codeBeforeMainLoop() const {
        return hasCodeBeforeMainLoop;
    }
    
    /// This process is in zero/non-zero element of array of modular interfaces
    bool isZeroElmtMIF() {return zeroElmtMIF;}
    bool isNonZeroElmtMIF() {return nonZeroElmtMIF;}
    
    /// Get string suffix for non-zero element of array of modular interfaces
    std::string getMifElmtSuffix() {
        return mifElmtSuffix;
    }
    
    /// The CTHREAD has @break or @continue in loop with wait(), 
    /// no join states into single state 
    bool isBreakInRemovedLoop() {
        return breakInRemovedLoop;
    }
    
    /// The process can be generated as single thread, i.e w/o state variable
    bool isSingleState() {
        // If all wait() have the same successor, that is single state thread
        // No wait(N) in single state thread allowed
        // If wait() located in inner loop, thread is not single state 
        return (!cthreadStates->hasWaitNState() && 
                waitSuccsSet.size() == 1 && waitMaxLoopLevel == 1);
    }
    
protected:
    /// Maximal iteration number analyzed for a loop
    unsigned LOOP_MAX_ITER = 10;
    /// Deep loop, 3rd and more nested loop
    unsigned DEEP_LOOP_DEPTH = 2;
    /// Maximal iteration number for a deep loop
    unsigned DEEP_LOOP_MAX_ITER = 3;
    // Iteration number to compare state to check stable in loop with 
    // unknown iteration number 
    unsigned COMPARE_STATE_ITER = 2;
    /// Iteration number when iteration exceeded error is reported 
    unsigned UNROLL_ERROR_ITER = 20;
    
    /// CFG fabric singleton
    CfgFabric* cfgFabric = CfgFabric::getFabric(astCtx);
    /// Level, sub-statement and other statement information provider
    ScStmtInfo stmtInfo;
    
    /// Current and previous CFG blocks in analysis
    AdjBlock block;
    AdjBlock prevBlock;
    /// Element index in CFG block
    size_t elemIndx; 
    /// Exit from function CFG block ID
    unsigned exitBlockId;
    
    /// ...
    sc_elab::VerilogModule* verMod = nullptr;
    /// Current function CFG
    clang::CFG* cfg;

    /// Delayed Clang CFG blocks with extended scope predecessors 
    std::list< std::pair<AdjBlock, std::vector<ConstScopeInfo> > >  delayed;
    
    /// Loop statement stack, last loop is most inner
    ConstLoopStack loopStack;
    /// Function directly called from this function <func expr, return value>
    std::unordered_map<clang::Stmt*, SValue>  calledFuncs;
    
    /// Functions with wait()
    std::unordered_set<const clang::FunctionDecl*>  hasWaitFuncs;
    
    /// Loops which contain wait()
    std::unordered_set<const clang::Stmt*>   waitInLoops;
    /// Visited loops, used to detect loop with wait() having combinational path
    std::unordered_set<const clang::Stmt*>   visitedLoops;
    
    /// States stored at wait() calls, used to register all reachable @wait()
    /// and get UseDef results from states, use map to keep it ordered
    /// <waitId, state>
    std::map<WaitID, ScState>  waitStates;
    /// Wait following statements to check for single state thread
    /// If there is only 1 element in this set, then thread is single state
    std::unordered_set<const clang::Stmt*>   waitSuccsSet;
    /// Maximal loop level of wait() statement, used to determine single state thread
    unsigned waitMaxLoopLevel = 1;

    /// Wait call argument in last statement: 0 for non-wait statement,
    /// 1 for wait(), N for wait(N)
    unsigned waitCall = 0;
    // Dead condition on path from &&/|| terminator to final IF/Loop/? condition, 
    // used to avoid dead condition generation
    bool deadCond = false;

    /// Function call in last statement
    bool funcCall = false;
    
    /// Context for last called function
    std::shared_ptr<ConstFuncContext> lastContext = nullptr;
    /// Call context stack
    ConstProcContext  contextStack;
    
    /// Live statements, to print in ScTraverseProc
    std::unordered_set<clang::Stmt*> liveStmts;
    /// Live terminators, also includes switch cases/default
    std::unordered_set<clang::Stmt*> liveTerms;
    /// Not mandatory required statements, can be removed in @removeUnusedStmt()
    std::unordered_set<clang::Stmt*> simpleStmts;
    /// Removed reference parameter arguments during CPA as unused code
    std::unordered_set<clang::Stmt*> removedArgExprs;
    
    /// Evaluated terminator condition values, use in ScTraverseProc
    std::unordered_map<CallStmtStack, SValue> termConds;
    /// Functions evaluated as constants if stored SValue is integer or 
    /// not eligible if NO_VALUE stored
    std::unordered_map<CallStmtStack, SValue> constEvalFuncs;

    /// CTHREAD wait states and constant propagation result providers
    ScCThreadStates* cthreadStates = nullptr;
    const FindWaitCallVisitor* findWaitInLoop = nullptr;
    /// There is a loop with wait() (means removed loop) with @break or @continue
    bool breakInRemovedLoop = false;
    
    /// Global module-level state, if constant propagation detects usage of
    /// global constant it need to put their values into global state, so they
    /// are there for Verilog generation
    std::shared_ptr<ScState> globalState;
    
    /// Final state used to get UseDef information
    std::shared_ptr<ScState> finalState = nullptr;

    /// Evaluate global constant to store in global state
    ScParseExprValue globExprEval;
    /// Elaboration database, used to create detected global variables
    sc_elab::ElabDatabase* elabDB = nullptr;
    /// Dynamic class module which is current for analyzed process
    SValue dynmodval;

    /// Sequential method process
    bool isSeqMethod;
    /// Current process has reset signal
    bool hasReset;
    /// Current process has code before main loop
    bool hasCodeBeforeMainLoop = false;
    /// Main loop terminator for CTHREAD
    const clang::Stmt* mainLoopStmt = nullptr;
    
    /// Current process is in zero/non-zero element of array of modular interfaces
    /// Used to suppress process variable generation
    bool zeroElmtMIF = false;
    bool nonZeroElmtMIF = false;
    /// String suffix for non-zero element of array of modular interfaces
    std::string mifElmtSuffix = "";
    
    /// Constants defined in reset section
    std::unordered_set<SValue> resetDefinedConsts;
    /// Accessed (read or/and defined) in CTHREAD reset section variables
    std::unordered_set<SValue> inResetAccessVars;
    /// Accessed (read or/and defined) after CTHREAD reset section variables
    std::unordered_set<SValue> afterResetAccessVars;
    /// Accessed in CTHREAD process SVA
    std::unordered_set<SValue> inSvaAccessVars;
};

}


#endif /* SCTRAVERSECONST_H */

