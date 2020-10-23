/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */
#include <sc_tool/cthread/ScSingleStateThread.h>
#include <sc_tool/cthread/ScCFGAnalysis.h>
#include <sc_tool/utils/CfgFabric.h>
#include <sc_tool/utils/ScTypeTraits.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/DeclCXX.h>
#include <unordered_set>

using namespace clang;

namespace sc {

namespace {

class SingleStateImp {
    const ScCThreadStates &states;
    ASTContext &astCtx;

    /// Set of all statements succeeding wait()s
    /// If there is only 1 element in this set, then thread has only 1 state
    std::unordered_set<const Stmt*> waitSuccsSet;

public:

    SingleStateImp (const ScCThreadStates &states)
    : states(states)
    , astCtx(states.getEntryFunction()->getASTContext())
    {}

    const Stmt * getNextStmt(const CFGBlock *curBlock, size_t nextElementID) {

        if (nextElementID == 0) {
            // we are entering new block, check if it is a loop header
            if (auto *loopStmt = getLoopStmt(curBlock))
                return loopStmt;
        }

        while (nextElementID < curBlock->size()) {
            CFGElement nextElement = (*curBlock)[nextElementID];
            assert(nextElement.getKind() == CFGElement::Statement);
            auto nextStmt = nextElement.getAs<CFGStmt>()->getStmt();

            if (auto expr = dyn_cast<Expr>(nextStmt);
                expr && expr->isIntegerConstantExpr(astCtx)) {
                ++nextElementID;
            }
            else {
                return nextStmt;
            }
        }

        if (nextElementID < curBlock->size()) {
            // current stmt is not last in block, pick next stmt
            CFGElement nextElement = (*curBlock)[nextElementID];
            assert(nextElement.getKind() == CFGElement::Statement);
            auto nextStmt = nextElement.getAs<CFGStmt>();
            return nextStmt->getStmt();
        }

        if (const Stmt* termStmt = curBlock->getTerminator().getStmt()) {
            return termStmt;
        }

        if (curBlock->succ_size() == 0)
            return nullptr;

        assert(curBlock->succ_size() == 1);

        // get first stmt of next block
        return getNextStmt(*curBlock->succ_begin(), 0);
    }


    bool run() {

        if (states.getNumFSMStates() == 0) {
            return true;
        }
        
        // Do not consider thread as single state if there is wait(n)
        if (states.hasWaitNState()) {
            return false;
        }

        for (size_t i = 0; i < states.getNumFSMStates(); ++i) {
            const auto waitStack = states.getStateCallStack(i);
            const auto waitCursor = waitStack.back();
            
//            waitCursor.getFuncDecl()->getBeginLoc().print(llvm::outs(),
//                astCtx.getSourceManager());
//            CFG* cfg = CfgFabric::get(waitCursor.getFuncDecl());
//            cfg->dump(astCtx.getLangOpts(), true);
            waitSuccsSet.insert(getNextStmt(waitCursor.getBlock(),
                waitCursor.getElementID() + 1));
        }

        assert(waitSuccsSet.size() > 0);
//        llvm::outs() << "waitSuccsSet size : " << waitSuccsSet.size() << "\n";
        return waitSuccsSet.size() == 1;
    }
};

} // namespace


bool isSingleStateThread(const ScCThreadStates &states)
{
    return SingleStateImp(states).run();
}

} // namespace sc