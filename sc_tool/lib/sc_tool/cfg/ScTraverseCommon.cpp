/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "sc_tool/cfg/ScTraverseCommon.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"

using namespace clang;
using namespace sc;

// For loop visitor instance
ForLoopVisitor ForLoopVisitor::signleton;

// Check if statement is member function of @sct_zero_width
bool sc::isZeroWidthCall(clang::Stmt* stmt)
{
    if (auto expr = dyn_cast<CXXMemberCallExpr>(stmt)) {
        Expr* thisExpr = expr->getImplicitObjectArgument();
        return (isZeroWidthType(thisExpr->getType()) || 
                isZeroWidthArrayType(thisExpr->getType()));
    }    
    return false;
}

// Check if statement is call expression of user defined function/method
bool sc::isUserCallExpr(clang::Stmt* stmt)
{
    SCT_TOOL_ASSERT (stmt, "Null statement");
    
    if (auto expr = dyn_cast<CallExpr>(stmt)) {
        // Get function name and type
        FunctionDecl* funcDecl = expr->getDirectCallee();
        SCT_TOOL_ASSERT (funcDecl, "No function found for call expression");
        std::string fname = funcDecl->getNameAsString();
        auto nsname = getNamespaceAsStr(funcDecl);

        // Functions from @sc_core:: and @sc_dt:: not analyzed, 
        // some functions from @std:: not analyzed
        if ((fname == "__assert" || fname == "__assert_fail") ||
            (nsname && (*nsname == "sc_core" || *nsname == "sc_dt")) ||
            (((nsname && *nsname == "std") || isLinkageDecl(funcDecl)) &&
               (fname == "printf" || fname == "fprintf" || 
                fname == "sprintf" || fname == "snprintf" ||
                fname == "fopen" || fname == "fclose" || 
                fname == "operator[]"))) {
            return false;
        }
        return true;
    }
    return false;
}

// Check if the FOR loop has externally declared counter
bool sc::hasForExtrCntr(clang::Stmt* stmt) 
{
    if (stmt == nullptr) return false;
    
    if (auto forStmt = dyn_cast<ForStmt>(stmt)) {
        return !ForLoopVisitor::get().hasInternalCntr(forStmt);
    }
    return false;
}

// Check if the block is loop entry
// Return loop statement if this block is loop entry or nullptr
const clang::Stmt* sc::checkBlockLoopEntry(AdjBlock block)
{
    // Entry block has no predecessors
    if (block.getCfgBlock()->pred_size() == 0) {
        return nullptr;
    }
    
    // First input of loop statement is input from loop body
    auto pred = AdjBlock::getCfgBlock( *(block.getCfgBlock()->preds().begin()) );
    const Stmt* loopTarget = (pred) ? pred->getLoopTarget() : nullptr;

    return loopTarget;
}

// Check if block with @do part of @do..@while and return terminator,
// the block has input from @do..@while direct or via empty block(s)
// \return termination or nullptr
const clang::DoStmt* sc::getDoWhileTerm(AdjBlock block)
{
    const Stmt* stmt = checkBlockLoopEntry(block);

    if (stmt) {
        if (auto doStmt = dyn_cast<const DoStmt>(stmt)) {
            return doStmt;
        }
    }
    return nullptr;
}

// Check if DoWhile body is empty
bool sc::isDoWhileEmpty(const clang::DoStmt* stmt) 
{
    const Stmt* doBody = stmt->getBody();
    if (!doBody) {
        SCT_INTERNAL_ERROR (stmt->getBeginLoc(), 
                            "No body statement in do...while loop");
    }

    if (auto doBodyComp = dyn_cast<CompoundStmt>(doBody)) {
        if (doBodyComp->body_empty()) {
            return true;
        }
    }
    
    return false;
}

// Check if the next block is loop input Phi function (with For/While term) and 
// current block inside of the loop body
// @param block -- current block 
// @param next -- next block which may contain loop statement terminator
bool sc::checkLoopInputFromBody(AdjBlock block, AdjBlock next) 
{
    // Get CFG block considering @next can be unreachable
    const CFGBlock* cfgBlock = next.getCfgBlock();
    
    if (cfgBlock == nullptr) {
        return false;
    }
    
    if (checkBlockLoopEntry(next)) {
        // First input of loop statement is input from loop body
        auto preds = cfgBlock->preds();
        // Cast to AdjBlock to correctly compare unreachable blocks
        if ( block == AdjBlock(*(preds.begin())) ) {
            return true;
        }
    }
    return false;
}

// Check if the next block is loop input Phi function (with For/While term) and 
// current block is input from outside
// @param block -- current block 
// @param next -- next block which may contain loop statement terminator
bool sc::checkLoopInputFromOut(AdjBlock block, AdjBlock next) 
{
    // Get CFG block considering @next can be unreachable
    const CFGBlock* cfgBlock = next.getCfgBlock();

    if (cfgBlock == nullptr) {
        return false;
    }
    
    if (checkBlockLoopEntry(next)) {
        // All input(s) except first are inputs from outside
        auto preds = cfgBlock->preds();
        auto p = preds.begin();
        for (++p; p != preds.end(); ++p) {
            // Cast to AdjBlock to correctly compare unreachable blocks
            if (block == AdjBlock(*p)) {
                return true;
            }
        }
    }
    return false;
}

// Get cases with their blocks, reordered in direct order
std::vector<std::pair<const clang::SwitchCase*, AdjBlock>> 
sc::getSwitchCaseBlocks(const clang::SwitchCase* swcase, 
                        clang::CFGBlock* switchBlock)
{
    using namespace std;
    // Case block predecessors
    std::vector<CFGBlock*> casePreds;
    std::vector<std::pair<const SwitchCase*, AdjBlock>> cases;

    // Try to find case block among switch block successors
    // Case block can be not found for switch with constant condition
    while (swcase) {
        AdjBlock caseBlock;

        for (auto succBlock : switchBlock->succs()) {
            AdjBlock b(succBlock);

            if (b.getCfgBlock() && b.getCfgBlock()->getLabel() == swcase) {
                caseBlock = b; break;
            }
        }
        
        // Fill case block predecessors to find all other case blocks
        if (caseBlock.getCfgBlock()) {
            casePreds.insert(casePreds.end(), 
                             caseBlock.getCfgBlock()->pred_begin(),
                             caseBlock.getCfgBlock()->pred_end());
        }
        
        cases.emplace_back(swcase, caseBlock);
        swcase = swcase->getNextSwitchCase();
    }
    
//    cout << "casePreds " << endl;
//    for (CFGBlock* b : casePreds) {
//        cout << "    " << (b ? b->getBlockID() : 0) << endl;
//    }
    
    // Look for not found blocks from case block predecessors
    for (auto& entry : cases) {
        if (!entry.second.getCfgBlock()) {
            for (auto pred : casePreds) {
                if (pred && pred->getLabel() == entry.first) {
                    entry.second = AdjBlock(pred, false);
                }
            }
        }
    }
    
    std::reverse(cases.begin(), cases.end());
    
    return cases;
}

// Check if block is case or default case block
bool sc::isCaseBlock(AdjBlock block) 
{
    Stmt* lblStmt = block.getCfgBlock() ? 
                    block.getCfgBlock()->getLabel() : nullptr;
    bool switchCase = lblStmt && isa<SwitchCase>(lblStmt);
    
    return switchCase;
}

// Check if case block is empty, has no statements, no terminator and
// next block is also case or default
bool sc::isCaseEmpty(AdjBlock block) 
{
    CFGBlock* cfgBlock = block.getCfgBlock();
    SCT_TOOL_ASSERT (cfgBlock, "Check empty for null block");
    
    // This block has no statement and terminator
    bool empty = cfgBlock && cfgBlock->empty() && 
                 !cfgBlock->getTerminator().getStmt();
    
    // Check next block is switch case or default
    if (empty) {
        bool first = true;
        for (auto nextBlock : cfgBlock->succs()) {
            Stmt* lblStmt = nextBlock ? nextBlock->getLabel() : nullptr;

            // Empty block can have only one successor 
            if (!first || !lblStmt || !isa<SwitchCase>(lblStmt)) {
                empty = false;
            }
            first = false;
        }
    }
            
    return empty;
}

// Check is switch default statement has no statements, that cannot be 
// determined from CFG block as empty block can be also for loops
bool sc::isDefaultEmpty(const SwitchCase* swcase) 
{
    if (auto defstmt = dyn_cast<const DefaultStmt>(swcase)) {
        bool empty = isa<NullStmt>(defstmt->getSubStmt());
        return empty;
    }
    return false;
}

// Get block predecessor number excluding &&/|| predecessors
unsigned sc::getPredsNumber(AdjBlock block) 
{
    CFGBlock* cfgBlock = AdjBlock::getCfgBlock(block);
    unsigned res = 0;
    
    for (auto i : cfgBlock->preds()) {
        const CFGBlock* predBlock = AdjBlock::getCfgBlock(i);
        const Stmt* term = predBlock->getTerminator().getStmt();
        
        // Skip binary operator &&\|| predecessors 
        if (term) {
            if (auto binstmt = dyn_cast<BinaryOperator>(term) ) {
                BinaryOperatorKind opcode = binstmt->getOpcode();
                SCT_TOOL_ASSERT (opcode == BO_LOr || opcode == BO_LAnd, 
                                 "getPredsNumber : Incorrect operator");
                continue;
            }
        }
        res += 1;
    }

    return res;
}

