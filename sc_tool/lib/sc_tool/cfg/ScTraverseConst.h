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
    // Scope level
    unsigned level;
    // Iteration counter, used in constant propagator
    unsigned counter;
    // Last iteration state
    std::shared_ptr<ScState> state;
    // Iteration number or 0 if unknown
    unsigned iterNumber;
    
    bool operator ==(const ConstLoopStackInfo& other) {
        return (stmt == other.stmt);
    }
    
    bool operator ==(const ConstLoopStackInfo& other) const {
        return (stmt == other.stmt);
    }
    
    ConstLoopStackInfo(const clang::Stmt* stmt_, unsigned level_, 
                       unsigned counter_, ScState* state_, 
                       unsigned iterNumber_) :
        stmt(stmt_), level(level_), counter(counter_), state(state_), 
        iterNumber(iterNumber_)
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

/// Predecessor @scope, @state and @level values to store in @delayes
struct ConstScopeInfo 
{
    std::shared_ptr<ScState> state;
    unsigned    level; 
    /// Previous block
    AdjBlock prevBlock;
    
    bool loopInputBody;
    bool loopInputOut;
    /// Loop statement stack, last loop is most inner
    ConstLoopStack loopStack;
    // Analysis after wait, break/continue in removed loop or unreachable code,
    /// used just for Phi-function readiness, no statement generated 
    /// in the path which is after wait only
    bool deadCode;
    
    /// Visited loops, used to detect loop with wait() having combinational path
    std::unordered_set<const clang::Stmt*>   visitedLoops;
    
    /// If the @prevScope is block with loop/break terminator it stored here, 
    /// otherwise @nullptr is stored
    const clang::Stmt* loopExit;
    /// Up scope level for block after @IF with empty branch, that is required
    /// if another branch contains @break, so there is no @Phi function before
    bool upLevel;
    /// Dead predecessor should be ignored in preparing next block
    /// Used for &&/|| dead branch leads final IF
    bool deadCond;
    
    ConstScopeInfo(std::shared_ptr<ScState> state_,
              unsigned level_,    
              const AdjBlock& prevBlock_,
              bool loopInputBody_,
              bool loopInputOut_,
              const ConstLoopStack& loopStack_,
              bool deadCode_,
              const std::unordered_set<const clang::Stmt*>&  visitedLoops_,
              const clang::Stmt* loopExit_ = nullptr,
              bool upLevel_ = false,
              bool deadCond_ = false
              ) : 
        state(state_),
        level(level_),
        prevBlock(prevBlock_),
        loopInputBody(loopInputBody_),
        loopInputOut(loopInputOut_),
        loopStack(loopStack_),
        deadCode(deadCode_),
        visitedLoops(visitedLoops_),
        loopExit(loopExit_),
        upLevel(upLevel_),
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
    /// Function level, called function has level = level+1
    unsigned level;
    /// Dead code flag
    bool deadCode;
    /// Parsed statement value stored, used to avoid re-parse sub-expression 
    /// in complex condition, important for sub-expressions with side effects
    std::unordered_map<clang::Stmt*, SValue>   stmtStoredValue;
    
    explicit ConstFuncContext(
                    const CfgCursor& callPoint_,
                    const SValue& returnValue_,
                    const SValue& modval_,
                    const SValue& recval_,
                    const std::list< std::pair<AdjBlock, 
                          std::vector<ConstScopeInfo> > >& delayed_,
                    const ConstLoopStack& loopStack_,
                    const std::unordered_map<clang::Stmt*, SValue>& calledFuncs_,
                    unsigned level_,
                    bool deadCode_, 
                    const std::unordered_map<clang::Stmt*, SValue>& storedValue_) :
            callPoint(callPoint_), returnValue(returnValue_), 
            modval(modval_), recval(recval_),
            delayed(delayed_), loopStack(loopStack_), 
            calledFuncs(calledFuncs_), level(level_), deadCode(deadCode_), 
            stmtStoredValue(storedValue_)
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
                CFGStmt* s = elm.getAs<CFGStmt>().getPointer();
                stmtStack.push_back(s->getStmt());
                
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
                             std::shared_ptr<ScState> globalState = nullptr,
                             sc_elab::ElabDatabase *elabDB = nullptr,
                             ScCThreadStates* cthreadStates_ = nullptr,
                             bool isCombProcess_ = false) :
        ScParseExprValue(context_, state_, modval_, false),
        cthreadStates(cthreadStates_), 
        globalState(globalState), 
        elabDB(elabDB), 
        dynmodval(modval_),
        isCombProcess(isCombProcess_)
    {
        state->getMostDerivedClass(dynmodval, dynmodval);
        SCT_TOOL_ASSERT (LOOP_LAST_ITER > LOOP_MAX_ITER, 
                         "Incorrect loop analysis constants");
        enableCheckAssert();    
    }

    /// @state deleted in @ScTraverse destructor
    virtual ~ScTraverseConst() {
    }
    
protected:
    /// Extract counter variable value for given FOR initialization sub-statement
    /// \return variable value or NO_VALUE
    SValue extractCounterVar(const clang::Stmt* stmt);
    
    /// Evaluate terminator condition if it is compile time constant
    void evaluateTermCond(const clang::Stmt* stmt, SValue& val);
    
    /// Evaluate loop iteration number from conditional expression
    llvm::Optional<unsigned> evaluateIterNumber(const clang::Stmt* stmt);
    
    /// Check if this loop needs compare state with last iteration state
    /// \param iterCntr -- number of analyzed iteration
    bool isCompareState(const clang::Stmt* stmt, unsigned maxIterNumber,
                        unsigned iterNumber, unsigned iterCntr);
    
    /// Check then/else branches are empty
    bool checkIfBranchEmpty(const clang::Stmt* branch);
    
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
    
    /// Function call expression
    void parseCall(clang::CallExpr* expr, SValue& val) override;

    /// Member function call expression
    void parseMemberCall(clang::CXXMemberCallExpr* expr, SValue& val) override;
    
    /// Choose and run DFS step in accordance with expression type.
    /// Remove sub-statements from generator
    void chooseExprMethod(clang::Stmt *stmt, SValue &val) override;
    
    /// Initialize analysis context at function entry
    void initContext();
    
    /// Restore analysis context with given one
    void restoreContext();

    /// Creates constant value in global state if necessary
    SValue parseGlobalConstant(const SValue &sVar) override;

    /// Register variables accessed in and after reset section,
    /// check read-not-defined is empty in reset
    void registerAccessVar(bool isResetSection, const clang::Stmt* stmt);

public:
    /// Preset CFG for run analysis
    void setFunction(const clang::FunctionDecl* fdecl);
    
    // Run analysis at function entry, runs once per process
    void run();
    
    /// Run for function declaration, the same as @setFunction() and @run()
    void run(const clang::FunctionDecl* fdecl);
    
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
    
    /// Get stored state at wait() calls
    const std::map<WaitID, ScState>& getWaitStates() const{
        return waitStates;
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
    
    /// Get final state with UseDef information
    const ScState* getFinalState() const {
        return finalState.get();
    }
    
    /// This process is in zero/non-zero element of array of modular interfaces
    bool isZeroElmtMIF() {return zeroElmtMIF;}
    bool isNonZeroElmtMIF() {return nonZeroElmtMIF;}
    
    /// Get string suffix for non-zero element of array of modular interfaces
    std::string getMifElmtSuffix() {
        return mifElmtSuffix;
    }
    
protected:
    /// Maximal iteration number analyzed for a loop
    unsigned LOOP_MAX_ITER = 16;
    /// Code of last iteration index stored in @counter, 
    /// used to mark last iteration after state  becomes stable
    unsigned LOOP_LAST_ITER = 1024;
    /// Deep loop, 3rd and more nested loop
    unsigned DEEP_LOOP_DEPTH = 3;
    /// Maximal iteration number for a deep loop
    unsigned DEEP_LOOP_MAX_ITER  = 3;
    unsigned COMPARE_STATE_ITER1 = 2;
    unsigned COMPARE_STATE_ITER2 = 11;
    /// Iteration number when iteration exceeded error is reported 
    unsigned UNROLL_ERROR_ITER = 20;
    
    /// CFG fabric singleton
    CfgFabric* cfgFabric = CfgFabric::getFabric(astCtx);
    
    /// Current and previous CFG blocks in analysis
    AdjBlock block;
    AdjBlock prevBlock;
    /// Element index in CFG block
    size_t elemIndx; 
    /// Exit from function CFG block ID
    unsigned exitBlockId;
    
    /// Current function 
    const clang::FunctionDecl* funcDecl = nullptr;
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

    /// Wait call argument in last statement: 0 for non-wait statement,
    /// 1 for wait(), N for wait(N)
    unsigned waitCall = 0;
    /// After wait, break/continue in removed loop and unreachable code, 
    /// no statement generated 
    /// Used to provide correct order of block analysis
    bool deadCode = false;
    // Dead condition on path from &&/|| terminator to final IF/Loop/? condition, 
    // used to avoid dead condition generation
    bool deadCond = false;

    /// Function call in last statement
    bool funcCall = false;
    /// Parsed statement value stored, used to avoid re-parse sub-expression 
    /// in complex condition, important for sub-expressions with side effects
    std::unordered_map<clang::Stmt*, SValue>   stmtStoredValue;
    
    /// Empty case blocks, used in @calcNextLevel()
    std::unordered_set<sc::AdjBlock> emptyCaseBlocks;

    /// Context for last called function
    std::shared_ptr<ConstFuncContext> lastContext = nullptr;
    /// Call context stack
    ConstProcContext  contextStack;
    
    /// Evaluated terminator condition values, use in ScTraverseProc
    std::unordered_map<CallStmtStack, SValue> termConds;

    /// THREAD wait states and constant propagation result providers
    ScCThreadStates* cthreadStates = nullptr;
    
    /// Global module-level state, if constant propagation detects usage of
    /// global constant it need to put their values into global state, so they
    /// are there for Verilog generation
    std::shared_ptr<ScState> globalState;
    
    /// Final state used to get UseDef information
    std::shared_ptr<ScState> finalState = nullptr;

    /// Elaboration database, used to create detected global variables
    sc_elab::ElabDatabase *elabDB = nullptr;
    /// Dynamic class module which is current for analyzed process
    SValue dynmodval;
    
    /// Current process is combinatorial
    bool isCombProcess;
    /// Current process has reset signal
    bool hasReset;
    /// Current process is in zero/.non-zero element of array of modular interfaces
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

