/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCCTHREADSTATE_H
#define SCTOOL_SCCTHREADSTATE_H

#include <sc_tool/elab/ScObjectView.h>
#include <sc_tool/cthread/ScCfgCursor.h>
#include <sc_tool/cfg/ScState.h>
#include "clang/AST/Decl.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <set>
#include <unordered_set>

namespace sc {

typedef int WaitID;

static constexpr WaitID RESET_ID = -1;

/// Contains call stacks for all wait states in CThread
/// Reset state is represented by empty call stack and has ID == RESET_ID
class ScCThreadStates 
{
    class ScCThreadStatesBuilder;
    friend class ScCThreadStatesBuilder;

    // Process function
    clang::FunctionDecl * entryFunction;
    /// Each wait() call state is identified by call stack
    /// There can be multiple entries for a single wait() call
    /// Last call in stack is wait()
    std::vector<CfgCursorStack> statesVec;

    /// Maximum N in wait(N) calls, counter will be 32bit for non-evaluable parameters
    size_t maxWaitN = 1;

    /// Set of all states with a counter ( wait(N) calls )
    std::unordered_set<WaitID> waitNSet;
    
    /// First @wait() is @wait(N), used to avoid reset WAIT_N counter in reset
    bool firstWaitN = false;

    /// Reset state has empty call stack
    inline static const CfgCursorStack resetStack = CfgCursorStack();

    const clang::ASTContext &astCtx;

public:

    ScCThreadStates(clang::FunctionDecl* entryFunction,
                    const clang::ASTContext& astCtx);

    /// Get number of states in CThread (w/o Reset state)
    size_t getNumFSMStates() const { return statesVec.size(); }

    /// Get ID of state
    WaitID getStateID(const CfgCursorStack& waitState) const ;

    /// Get ID of state, create new state if not exists
    /// \param waitState - call stack for wait call
    /// \param maxWaitCycles - Maximum N for wait(N)
    WaitID getOrInsertStateID(const CfgCursorStack& waitState,
        size_t maxWaitCycles = 1);

    bool hasWaitNState() const;

    size_t getMaxWaitN() const;

    /// Returns true for wait(N > 1)
    bool isWaitNState(WaitID wID) const;
    
    bool isFirstWaitN() const;

    CfgCursorStack getStateCallStack(WaitID stateID) const {
        if (stateID == RESET_ID)
            return resetStack;
        return statesVec.at(stateID);
    }

    clang::FunctionDecl *getEntryFunction() const { return entryFunction; }
};

struct EvalResult {
    bool isUnknown;
    llvm::APSInt value;

    EvalResult(const sc::SValue &val)
    : isUnknown(!val.isInteger())
    , value( isUnknown ? llvm::APInt() : val.getInteger() )
    {}

    EvalResult() : isUnknown(true) {}
    EvalResult(llvm::APSInt val) : isUnknown(false), value(val) {}

    bool operator == (const EvalResult &other) const {
        if (isUnknown && other.isUnknown)
            return true;

        return (isUnknown == other.isUnknown) && (value == other.value);
    }

    bool operator !=(const EvalResult &other) {
        return !(*this == other);
    }
};

struct EvaluatedStmt {

    EvaluatedStmt(CfgCursorStack callStack, const clang::Stmt *expr)
    : callStack(std::move(callStack)), stmt(expr)
    {}

    const CfgCursorStack callStack;
    const clang::Stmt *stmt;

    bool operator == (const EvaluatedStmt& other) const {
        bool res = (callStack == other.callStack)
                && (stmt == other.stmt);
        return res;
    }

};

typedef std::vector<SValue> ModValStack;

/// Information about state gathered during constant propagation
///  - Reachable states
///  - Values of evaluated expressions
///  - SRecords for call stack for each reachable state
class ScThreadStateInfo {
    WaitID waitID = 0;
    std::set<WaitID> reachableIDs;
    std::unordered_map<WaitID, ModValStack> stateModValStackMap; // Not required?

    // TODO: convert to hash map
    std::vector<std::pair<EvaluatedStmt, EvalResult>> stmtVals;
public:
    explicit ScThreadStateInfo (WaitID waitID) : waitID(waitID) {}
    explicit ScThreadStateInfo () = default;

    /// @return ID of wait state
    WaitID startStateID () const { return waitID; }

    /// @return set of wait state IDs reachable from this state
    const std::set<WaitID> &reachableStateIDs() const {
        return reachableIDs;
    }

    /// check if waitID is reachable from current wait state
    bool isReachable(WaitID waitID) const;

    /// @return evaluated result for given expression
    EvalResult getBranchStmtValue(EvaluatedStmt branchStmt) const;

    /// Set WaitID as reachable from current wait state
    void addReachableWait(WaitID id, const ModValStack& modValStack)
    {
        reachableIDs.insert(id);
        stateModValStackMap.emplace(id, modValStack);
    }

    ModValStack getModValStackForState(WaitID id) const
    {
        return stateModValStackMap.at(id);
    }

    void addEvaluatedStmt(EvaluatedStmt stmt, EvalResult result);

    template <typename StreamT>
    friend StreamT &operator<<(StreamT &os, const ScThreadStateInfo &sInfo);

};

template<typename StreamT>
StreamT& operator<<(StreamT &os, const ScThreadStateInfo &sInfo)
{
    os << "Wait # " << sInfo.startStateID() << " static info:\n";
    os << " Reachable states: \n";
    for (auto stateID : sInfo.reachableIDs) {
        os << "  # " << stateID << "\n";
        os << "     ModVal stack: ";
        for (const auto & sval : sInfo.getModValStackForState(stateID))
            os << "->" << sval;

        os << "\n";
    }

    os << " Evaluated stmts: \n";
    for (auto evalStmt : sInfo.stmtVals) {
        os <<"  "<< evalStmt.first.stmt->getStmtClassName();

        if (evalStmt.second.isUnknown)
            os << " UNKWN\n";
        else
            os << " " << evalStmt.second.value.getZExtValue() << "\n";
    }


    return os;
}

} // namespace sc


#endif //SCTOOL_SCCTHREADSTATE_H
