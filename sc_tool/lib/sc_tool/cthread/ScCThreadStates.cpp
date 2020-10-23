/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "ScCThreadStates.h"
#include <sc_tool/elab/ScElabDatabase.h>
#include <sc_tool/utils/ScTypeTraits.h>
#include <sc_tool/utils/CfgFabric.h>

using namespace llvm;
using namespace clang;

namespace sc {

ScCThreadStates::ScCThreadStates(clang::FunctionDecl * entryFunction,
                                 const clang::ASTContext &astCtx)
    : entryFunction(entryFunction)
    , astCtx(astCtx)
{}

WaitID ScCThreadStates::getStateID(const CfgCursorStack &waitState) const {
    size_t waitID = 0;

    for (auto &state : statesVec) {
        if (waitState == state) {
            return waitID;
        }
        waitID ++;
    }
    llvm_unreachable("can't find state id");
    return 0;
}

bool ScCThreadStates::hasWaitNState() const {
    return !waitNSet.empty();
}

size_t ScCThreadStates::getMaxWaitN() const {
    return maxWaitN;
}

bool ScCThreadStates::isWaitNState(WaitID wID) const
{
    return waitNSet.count(wID) == 1;
}

WaitID ScCThreadStates::getOrInsertStateID(const CfgCursorStack &waitState,
                                           size_t maxWaitCycles)
{
    if (resetStack == waitState)
        return RESET_ID;

    WaitID waitID = 0;

    for (auto &state : statesVec) {
        if (waitState == state) {
            return waitID;
        }
        waitID ++;
    }

    statesVec.emplace_back(waitState);

    if (maxWaitCycles > 1) {
        maxWaitN = std::max(maxWaitN, maxWaitCycles);
        waitNSet.insert(waitID);
    }

    return waitID;
}

// =============================================================================
// =============================================================================


EvalResult
ScThreadStateInfo::getBranchStmtValue(EvaluatedStmt branchStmt) const
{
    auto it =
    std::find_if(stmtVals.cbegin(), stmtVals.cend(), [&](auto e) {
        return e.first == branchStmt;
    });

    if (it == stmtVals.cend()) {
        return EvalResult{};
    } else
        return it->second;
}

bool ScThreadStateInfo::isReachable(WaitID waitID) const {
    return reachableIDs.count(waitID);
}

void ScThreadStateInfo::addEvaluatedStmt(EvaluatedStmt stmt, EvalResult result)
{
    auto it = std::find_if(stmtVals.begin(), stmtVals.end(), [&](auto e) {
            return e.first == stmt;
        });

    if (it == end(stmtVals)) {
        stmtVals.push_back(std::make_pair(stmt, result));
    } else if ( !(it->second == result) )
        it->second.isUnknown = true;

}

} // namespace sc