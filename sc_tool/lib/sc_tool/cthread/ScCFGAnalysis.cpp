/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "ScCFGAnalysis.h"
#include <sc_tool/diag/ScToolDiagnostic.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using namespace llvm;
using namespace clang;

namespace {

struct ColoredBlock {
    const CFGBlock *block;
    unsigned color;

    bool operator == (const ColoredBlock &other) const {
        return (block == other.block) && (color == other.color);
    }

};

} // anonymous namespace


namespace std
{

template<>
struct hash<ColoredBlock>
{
    std::size_t
    operator()(const ColoredBlock &obj) const
    {
        return llvm::hash_combine(obj.color, obj.block);
    }

};

} // namespace std




namespace sc {

namespace {

typedef std::unordered_set<const CFGBlock *> BlockSet;
typedef std::unordered_map<const CFGBlock *, const CFGBlock *> BlockExitMap;

class FindCondExitBlock {


    typedef unsigned BlockColor;
    std::unordered_map<const CFGBlock *, std::unordered_set<BlockColor>> blockColors;

    std::unordered_set<ColoredBlock> visitedBlocks;

    std::queue<ColoredBlock> blockQueue;

    unsigned numRootColors;

    BlockSet *entryBlockSet = nullptr;
    BlockExitMap *exitMap = nullptr;
    const CFGBlock * exitBlock = nullptr;

    void traverseBlock(ColoredBlock coloredBlock) {

        auto* block = coloredBlock.block;

        if (block->hasNoReturnElement()) {
            ScDiag::reportScDiag(ScDiag::SC_ERROR_NORETURN);
            SCT_INTERNAL_FATAL_NOLOC ("Functions with NORETURN attribute");
        }

        bool alreadyVisited = visitedBlocks.count(coloredBlock);
        visitedBlocks.insert(coloredBlock);

        blockColors[block].insert(coloredBlock.color);

        if (blockColors[block].size() == numRootColors) {
            exitBlock = block;
            return;
        }

        if (alreadyVisited)
            return;

        size_t numReachableSuccs = 0;
        for (auto succ: block->succs())
            if (succ.isReachable())
                ++numReachableSuccs;


        auto termStmt = block->getTerminator().getStmt();

        if (numReachableSuccs > 1 && isCondStmt(termStmt) ) {
            auto *subExitBlock = FindCondExitBlock().findExit(block, entryBlockSet, exitMap);
            //llvm::outs() << "   next " << subExitBlock << "\n";
            if (subExitBlock)
                blockQueue.push({subExitBlock, coloredBlock.color});
        } else {

            for (auto succ: block->succs()) {
                if (succ.isReachable()) {
                    //llvm::outs() << "   next " << (CFGBlock*)succ << "\n";
                    blockQueue.push({succ, coloredBlock.color});
                }
            }

        }



    }



public:

    /// If entryBlock terminator is conditional stmt (if or switch),
    /// returns first block after conditional body
    const CFGBlock * findExit(const CFGBlock *entryBlock,
                              BlockSet *initEntries,
                              BlockExitMap *initExitMap)
    {

        //llvm::outs() << "find exit block: " << entryBlock->getBlockID() << "\n";

        entryBlockSet = initEntries;
        exitMap = initExitMap;

        // entry already visited
        if (entryBlockSet->count(entryBlock)) {
            if (exitMap->count(entryBlock))
                return (*exitMap)[entryBlock];

            return nullptr;
        }

        entryBlockSet->emplace(entryBlock);

        exitBlock = nullptr;

        const Stmt *termStmt = entryBlock->getTerminator().getStmt();
        if (isCondStmt(termStmt)) {

            numRootColors = 0;

            for (auto succ: entryBlock->succs()) {
                if (succ.isReachable()) {
                    blockQueue.push({succ, numRootColors});
                    ++numRootColors;
                }
            }

            while (!blockQueue.empty() && !exitBlock) {

                auto blockToEval = blockQueue.front();
                blockQueue.pop();

                traverseBlock(blockToEval);

            }
        }

        if (exitBlock)
            exitMap->emplace(entryBlock, exitBlock);

        return exitBlock;
    }

};


} // end anonymous namespace

const clang::CFGBlock *getCondExitBlock(const clang::CFGBlock *entryBlock) {

    BlockSet entries;
    BlockExitMap exitMap;

    auto exitBlock = FindCondExitBlock().findExit(entryBlock, &entries, &exitMap);

//    if (!exitBlock) {
//        llvm::outs() << "CANT FIND COND EXIT BLOCK \n";
//    } else
//        llvm::outs() << "COND EXIT BLOCK #"  << exitBlock->getBlockID() << "\n";

    return exitBlock;
}

const clang::CFGBlock *
getCondOrLoopExitBlock(const clang::CFGBlock *entryBlock,
                       const clang::Stmt *headerStmt ) {

    if (isLoopStmt(headerStmt)) {
        // search while block for do..while
        if (isa<DoStmt>(headerStmt)
            && (headerStmt != entryBlock->getTerminator().getStmt())) {
            // go backwards to find while()
            const clang::CFGBlock *backEdge = getLoopBackEdge(entryBlock);
            assert(backEdge->pred_size() == 1);
            const clang::CFGBlock *whileBlock = *backEdge->pred_begin();
            entryBlock = whileBlock;
        }

        assert(entryBlock->succ_size() == 2);

        // pick second successor
        auto succIter = entryBlock->succ_begin();
        return *(++succIter);
    }
    else if (isCondStmt(headerStmt)) {
        return getCondExitBlock(entryBlock);
    } else {
        assert(false);
        return nullptr;
    }
}

const clang::CFGBlock *getLoopBackEdge(const clang::CFGBlock *loopHeaderBlock) {
    for (const CFGBlock::AdjacentBlock &pred : loopHeaderBlock->preds()) {
        const CFGBlock *block = pred;
        if (block && block->getLoopTarget())
            return block;
    }

    return nullptr;
}


const clang::Stmt* getLoopStmt(const clang::CFGBlock *loopHeaderBlock) {

    if (const CFGBlock *backEdgeBlock = getLoopBackEdge(loopHeaderBlock) )
        return backEdgeBlock->getLoopTarget();

    return nullptr;
}

bool isLoopHeaderBlock(const clang::CFGBlock *cfgBlock)
{
    return (getLoopStmt(cfgBlock) != nullptr);
}


bool isLoopStmt(const clang::Stmt *stmt) {
    return stmt && (   isa<ForStmt>(stmt)
                    || isa<WhileStmt>(stmt)
                    || isa<DoStmt>(stmt));
}

bool isCondStmt(const clang::Stmt *stmt) {
    return stmt && (isa<IfStmt>(stmt) || isa<SwitchStmt>(stmt) ||
        isa<ConditionalOperator>(stmt) || isa<BinaryOperator>(stmt));
}


bool isLoopBackEdge(const clang::CFGBlock *block) {
    return block->getLoopTarget();
}

namespace  {

} // end anonymous namespace


} // end namespace sc
