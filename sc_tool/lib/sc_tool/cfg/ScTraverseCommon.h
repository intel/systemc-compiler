/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SC process function traverse common classes.
 *  
 * File:   ScTraverseBase.h
 * Author: Mikhail Moiseev
 */

#ifndef SCTRAVERSEBASE_H
#define SCTRAVERSEBASE_H

#include "sc_tool/diag/ScToolDiagnostic.h"
#include "clang/Analysis/CFG.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include "clang/AST/Stmt.h"
#include <iostream>
#include <unordered_set>
#include <map>

namespace sc {
    
/// Adjacent block with override operator ==(), which considers reachable
/// and unreachable blocks with the same block ID as equal
class AdjBlock : public clang::CFGBlock::AdjacentBlock
{
public:
    AdjBlock() : 
        clang::CFGBlock::AdjacentBlock(NULL, false)
    {}

    AdjBlock(const clang::CFGBlock::AdjacentBlock& block) :
        clang::CFGBlock::AdjacentBlock((block.isReachable()) ? 
            block.getReachableBlock() : block.getPossiblyUnreachableBlock(), 
            block.isReachable())
    {}
    
    AdjBlock(clang::CFGBlock *B, bool IsReachable) :
        clang::CFGBlock::AdjacentBlock(B, IsReachable)
    {}
    
    /// Return CFG block, which can be NULL as well
    clang::CFGBlock* getCfgBlock() const {
        return (isReachable()) ? 
                getReachableBlock() : getPossiblyUnreachableBlock();
    }

    /// Return CFG block, which can be NULL as well
    static clang::CFGBlock* getCfgBlock(const clang::CFGBlock::AdjacentBlock& B) {
        return (B.isReachable()) ? 
                B.getReachableBlock() : B.getPossiblyUnreachableBlock();
    }

    /// Return block ID or maximal unsigned if block is NULL
    unsigned getCfgBlockID() const {
        return ((getCfgBlock()) ? getCfgBlock()->getBlockID() : UINT_MAX);
    }
    
    bool operator ==(const AdjBlock& other) {
        return (getCfgBlockID() == other.getCfgBlockID());
    }

    bool operator !=(const AdjBlock& other) {
        return (getCfgBlockID() != other.getCfgBlockID());
    }

    /// Get pointer to reachable or unreachable CFG block 
    /// Override AdjacentBlock::operator CFGBlock*(), 
    /// required for complex conditions in loop with constant, 
    /// loop body block can be unreachable from loop terminator
    operator clang::CFGBlock*() const {
        return getCfgBlock();
    }
};
    
/// Function call statement stack, used for constant propagation evaluated 
/// condition value
typedef std::vector<const clang::Stmt*> CallStmtStack;

}

//=============================================================================

namespace std {

// Hash function for @AdjacentBlock
template<> 
struct hash<sc::AdjBlock>  
{
    std::size_t operator () (const sc::AdjBlock& obj) const {
        using std::hash;
        return ( std::hash<unsigned>()(obj.getCfgBlockID()) );
    }
};

// Hash function for @CallStmtStack
template<> 
struct hash<sc::CallStmtStack>  
{
    std::size_t operator () (const sc::CallStmtStack& obj) const {
        using std::hash;
        size_t res = 0;
        for (auto i : obj) {
            res += std::hash<const void*>()(i);
        }
        return res;
    }
};
}

//=============================================================================

namespace sc {
    
/// Check if FOR loop uses internally declared counter 
class ForLoopVisitor : public clang::RecursiveASTVisitor<ForLoopVisitor> 
{
protected :
    clang::VarDecl* initVar = nullptr;
    clang::VarDecl* incVar = nullptr;
    
    static ForLoopVisitor signleton;

    ForLoopVisitor() {}
    
public:
    
    static ForLoopVisitor& get() {
        return signleton;
    }
    
    bool VisitStmt(clang::Stmt* stmt) 
    {
        using namespace clang;
        if (!initVar) {
            if (DeclStmt* declStmt = dyn_cast<DeclStmt>(stmt)) {
                SCT_TOOL_ASSERT(declStmt->getSingleDecl(), "No single declaration");
                
                if (VarDecl* varDecl = dyn_cast<VarDecl>(declStmt->getSingleDecl())) {
                    initVar = varDecl;
                }
            }
        }
        if (!incVar) {
            if (DeclRefExpr* refExpr = dyn_cast<DeclRefExpr>(stmt)) {
                SCT_TOOL_ASSERT(refExpr->getDecl(), "No single declaration");
                
                if (VarDecl* varDecl = dyn_cast<VarDecl>(refExpr->getDecl())) {
                    incVar = varDecl;
                }
            }
        }
        return true;
    }

    /// Return true if the loop uses internal counter or have no increment
    bool hasInternalCntr(clang::ForStmt* stmt) 
    {
        initVar = nullptr; 
        this->TraverseStmt(stmt->getInit());
        incVar = nullptr;
        this->TraverseStmt(stmt->getInc());
        
        return (initVar == incVar);
    }
    
    clang::VarDecl* getCounterDecl(clang::ForStmt* stmt) 
    {
        initVar = nullptr; 
        this->TraverseStmt(stmt->getInit());
        return initVar;
    }
};

/// Check if statement is member function of @sct_zero_width
bool isZeroWidthCall(clang::Stmt* stmt);

/// Check if statement is call expression of user defined function/method
bool isUserCallExpr(clang::Stmt* stmt);

/// Check if the FOR loop has externally declared counter
bool hasForExtrCntr(clang::Stmt* stmt);

/// Check if the block is loop entry
/// Return loop statement if this block is loop entry or nullptr
const clang::Stmt* checkBlockLoopEntry(AdjBlock block);

/// Check if block with @do part of @do..@while and return terminator,
/// the block has input from @do..@while direct or via empty block(s)
/// \return termination or nullptr
const clang::DoStmt* getDoWhileTerm(AdjBlock block);

/// Check if DoWhile body is empty
bool isDoWhileEmpty(const clang::DoStmt* stmt);

/// Check if the next block is loop input Phi function (with For/While term) and 
/// current block inside of the loop body
/// @param block -- current block 
/// @param next -- next block which may contain loop statement terminator
bool checkLoopInputFromBody(AdjBlock block, AdjBlock next);

/// Check if the next block is loop input Phi function (with For/While term) and 
/// current block is input from outside
/// @param block -- current block 
/// @param next -- next block which may contain loop statement terminator
bool checkLoopInputFromOut(AdjBlock block, AdjBlock next);

/// Get cases with their blocks, reordered in direct order
std::vector<std::pair<const clang::SwitchCase*, AdjBlock>> 
getSwitchCaseBlocks(const clang::SwitchCase* swcase, clang::CFGBlock* block);

/// Check if block is case or default case block
bool isCaseBlock(AdjBlock block);

/// Check if case block is empty, has no statements, no terminator and
/// next block is also case or default
bool isCaseEmpty(AdjBlock block);

/// Check is switch default statement has no statements, that cannot be 
/// determined from CFG block as empty block can be also for loops
bool isDefaultEmpty(const clang::SwitchCase* swcase);

/// Get block predecessor number excluding &&/|| predecessors
unsigned getPredsNumber(AdjBlock block);

//=============================================================================

/// Update predecessor scopes and return loop stack for the next block
template <typename LS, typename SI>
LS getLoopStack(std::vector<SI>& scopeInfos)
{
    using namespace clang;
    using namespace llvm;
    using namespace std;

    LS loopStack;
    
    // Remove last loop stack entry for branch with @loopExit
    for (auto& si : scopeInfos) {
        if (si.loopExit) {
            SCT_TOOL_ASSERT (si.loopStack.size(),
                             "Loop stack empty for branch with loop exit");
            SCT_TOOL_ASSERT (si.loopStack.back().stmt == si.loopExit,
                             "Loop stack back not equal to loop exit");
            si.loopStack.pop_back();
        }
    }
    
    // Get minimal loop stack 
    bool first = true;
    for (const auto& si : scopeInfos) {
        if (first || si.loopStack.size() < loopStack.size()) {
            loopStack = si.loopStack;
        }
        first = false;
    }

    // Correct loop exit for predecessor with stacks bigger than minimal one
    for (auto& si : scopeInfos) {
        if (si.loopStack == loopStack) continue;
        
        if (si.loopStack.size()-loopStack.size() == 1) {
            si.loopExit = si.loopStack.back().stmt;
            si.loopStack.pop_back();
            
            SCT_TOOL_ASSERT (si.loopStack == loopStack, 
                             "Different loop stacks in predecessors");
            // Loop stack correction normally required for switch case only
            if (!isa<const SwitchStmt>(si.loopExit)) {
                ScDiag::reportScDiag(si.loopExit->getBeginLoc(), 
                                     ScDiag::SYNTH_UNSUPPORTED_RETURN);
            }
        } else {
            SCT_TOOL_ASSERT (false, "Incorrect loop stack size");
        }
    }
    
    return loopStack;
}


/// Calculate next block level based on predecessor info
/// Used in ScTraverseProc and ScTraverseConst
/// \param switchCase -- next block is switch case first block
/// \return <next level, up level>
template <typename SI>
std::pair<unsigned, unsigned> calcNextLevel(
                            const std::vector<SI>& scopeInfos, 
                            unsigned currLoopLevel,
                            const std::unordered_set<AdjBlock>& emptyCaseBlocks,
                            bool switchCase = false)  
{
    using namespace clang;
    using namespace llvm;
    using namespace std;
    
    /*cout << "Level in preds: \n";
    for (const auto& si : scopeInfos) {
        cout << si.level;
        if (si.loopExit) cout << "(" << hex << si.loopExit << dec << ")";
        cout << ", B"<< si.prevBlock.getCfgBlockID() << " is loop entry " 
            << hex << checkBlockLoopEntry(si.prevBlock) << ", loop stack term "
             << (si.loopStack.empty() ? 0 : si.loopStack.back().stmt) << dec << endl;
    }*/
    
    // There is predecessor block with @continue
    bool continuePred = false;
    for (const auto& si : scopeInfos) {
        auto* term = si.prevBlock.getCfgBlock()->getTerminator().getStmt();
        continuePred = continuePred || (term && isa<ContinueStmt>(term));
    }
    
    // There is predecessor block with @return statement
    bool returnPred = false;
    for (const auto& si : scopeInfos) {
        // Get last element where return statement can be located
        auto* cfgBlock = si.prevBlock.getCfgBlock();
        size_t elmSize = cfgBlock->size();
        if (elmSize == 0) continue;
        
        const CFGElement& elm = cfgBlock->operator [](elmSize-1);
        
        // Check if last element is return statement
        if (elm.getKind() == CFGElement::Kind::Statement) {
            CFGStmt cfgstmt = elm.getAs<CFGStmt>().getValue();
            if (isa<const ReturnStmt>(cfgstmt.getStmt())) {
                returnPred = true; break;
            }
        }
    }
    
    // Predecessor level grouped by loop exit <loop exit, level>
    unordered_map<const Stmt*, unsigned>  predLoops;
    // Level with predecessor numbers <level, number>, sorted by level
    map<unsigned, unsigned>  levelNums;
    // If single predecessor, empty switch case is considered to determine level
    bool singlePred = scopeInfos.size() == 1;

    for (const auto& si : scopeInfos) {
        // Skip empty case and default case blocks
        if (!singlePred && emptyCaseBlocks.count(si.prevBlock) != 0)
            continue;
            
        if (si.loopExit) {
            auto i = predLoops.find(si.loopExit);
            if (i == predLoops.end()) {
                predLoops.emplace(si.loopExit, si.level);
            }
        } else {
            // Put predecessor with NO loop exit
            auto i = levelNums.find(si.level);
            if (i == levelNums.end()) {
                levelNums.emplace(si.level, 1);
            } else {
                i->second++;
            }
        }
    }

    //cout << "levelNums " << levelNums.size() << " predLoops " << predLoops.size() << endl;
    
    // Put predecessor with loop exit
    for (const auto& pl : predLoops) {
        auto i = levelNums.find(pl.second);
        if (i == levelNums.end()) {
            levelNums.emplace(pl.second, 1);
        } else {
            i->second++;
        }
    }
    
    // Minimal level is the first element level in map
    unsigned minLevel = levelNums.begin()->first;
    // maximal level is the last element level in map
    unsigned maxLevel = levelNums.rbegin()->first;
    //cout << "minLevel " << minLevel << ", maxLevel " << maxLevel << endl;

    // Calculate final level as sum of levels with predecessor numbers
    unsigned levelSum = levelNums.begin()->second;
    //cout << "Level numbers : " << minLevel << " = " << levelSum << ", ";

    for (unsigned l = minLevel+1; l < maxLevel+1; ++l) {
        auto i = levelNums.find(l);
        unsigned predNum = (i != levelNums.end()) ? i->second : 0;
        //cout << l << " = " << predNum << ", ";
        
        // Each level has its weight as 2^l, where l=0 for maximal level
        levelSum = 2*levelSum + predNum;
    }
    //cout << endl << "levelSum " << levelSum << endl;

    // Level up from minimal level
    unsigned upLevel = 0;
    // If level up is required by empty IF else branch 
    if (levelSum == 1 && scopeInfos.back().upLevel) {
        //cout << "Set upLevel 1 for empty else branch" << endl;
        upLevel = 1;
    }
    // Several inputs possible from loop exit, it is normal Phi-function 
    // Skip for @switchCase as it can have input from previous empty case
    if (!switchCase && levelSum > 1) {
        // Round up 
        upLevel = ceil(float(levelSum) / float(1 << (maxLevel-minLevel+1)));
    }

    SCT_TOOL_ASSERT(!continuePred || !returnPred, 
                    "Predecessors with continue and return statements");
        
    // Next block level
    unsigned nextLevel;
    if (continuePred) {
        // If there is input from @continue, then use current loop level
        nextLevel = currLoopLevel;
        //cout << "  predecessor with @continue, final level " << nextLevel << endl;
        
    } else 
    if (returnPred) {
        // Block with return is predecessor of last block of the function 
        // where all exits joined
        nextLevel = 0;
        //cout << "  predecessor with @return, final level 0"  << endl;
        
    } else {
        // Correct level, but not less than zero
        if (minLevel < upLevel) {
            cout << "Incorrect levels: minLevel " << minLevel << " upLevel " << upLevel << endl;
        }
        SCT_TOOL_ASSERT (minLevel >= upLevel, "Next level less than up level");
        
        nextLevel = (minLevel >= upLevel) ? (minLevel - upLevel) : 0;
        //cout << "  upLevel " << upLevel << ", final level "<< nextLevel << endl;
    }
    
    return pair<unsigned, unsigned>(nextLevel, upLevel);
}

}

#endif /* SCTRAVERSEBASE_H */

