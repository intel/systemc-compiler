/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "sc_tool/cfg/ScTraverseProc.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/CfgStmt.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/StringFormat.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/ScCommandLine.h"
#include "sc_tool/utils/CppTypeTraits.h"

#include "clang/AST/Decl.h"
#include <memory>

using namespace std;
using namespace clang;
using namespace llvm;

namespace sc {
    
// ---------------------------------------------------------------------------
// Auxiliary functions

// Store statement string for @stmt
void ScTraverseProc::storeStmtStr(Stmt* stmt) 
{
    if (auto str = codeWriter->getStmtString(stmt)) {
        scopeGraph->storeStmt(stmt, str.getValue());
    }
}

// Store statement string for @nullptr
void ScTraverseProc::storeStmtStrNull(Stmt* stmt) 
{
    if (auto str = codeWriter->getStmtString(stmt)) {
        scopeGraph->storeStmt(nullptr, str.getValue());
    }
}

// Set @noRemoveStmt to do not remove sub-statement in scope graph
void ScTraverseProc::setNoRemoveStmt(bool flag)
{
    noRemoveStmt = flag;
}

// Create new code scope
shared_ptr<CodeScope> ScTraverseProc::createScope() 
{
    return make_shared<CodeScope>();
}

// Get code scope for block or create new one
shared_ptr<CodeScope> ScTraverseProc::getScopeForBlock(AdjBlock block) 
{
    auto i = delayed.rbegin();
    for (; i != delayed.rend(); ++i) {
        if (i->first == block) {
            SCT_TOOL_ASSERT (!i->second.empty(), "No scope for block");
            return (i->second.front().currScope);
        }
    }
    return createScope();
}

// Generate temporal assertion inside of loop(s) if required
llvm::Optional<std::string> 
ScTraverseProc::getSvaInLoopStr(const std::string& svaStr, bool isResetSection) 
{
    std::string tabStr = (isResetSection ? "    " : "        ");
    std::string loopStr;
    
    for (auto i = loopStack.begin(); i != loopStack.end(); ++i) {
        if (!isa<const ForStmt>(i->stmt)) {
            ScDiag::reportScDiag(i->stmt->getBeginLoc(), 
                                 ScDiag::SYNTH_SVA_INCORRECT_LOOP);
            return llvm::None;
        }
        // Get terminator condition value for all/first iteration from CPA
        SValue termCValue; SValue termCValueFirst;
        getTermCondValue(i->stmt, termCValue, termCValueFirst);

        if (auto str = parseTerm(i->stmt, termCValue)) {
            loopStr += tabStr + *str + " begin\n";
        } else {
            return llvm::None;
        }
        tabStr += "    ";
    }
    
    loopStr += tabStr + (assertName ? (*assertName+" : ") : "") +
               "assert property ( " + svaStr + " );\n";
    
    for (size_t i = 0; i != loopStack.size(); ++i) {
        tabStr = tabStr.substr(0, tabStr.size()-4);
        loopStr += tabStr + "end\n";
    }

    return loopStr;
}

// Check if function has wait() inside from CPA stored in @hasWaitFuncs
bool ScTraverseProc::isWaitInFunc(const clang::FunctionDecl* decl) 
{
    return hasWaitFuncs.count(decl);
}

// Check if current function has wait() inside
bool ScTraverseProc::isWaitInCurrFunc() 
{
    return isWaitInFunc(funcDecl);
}
    
// Get terminator condition from CPA stored in #termConds
void ScTraverseProc::getTermCondValue(const Stmt* stmt, SValue& val, SValue& fval) 
{
    auto callStack = contextStack.getStmtStack();
    callStack.push_back(stmt);
    auto i = termConds.find(callStack);
    bool otherIters = (i != termConds.end());
    val = otherIters ? i->second : NO_VALUE;

    // Use double #term to distinguish first iteration for FOR/WHILE loops 
    callStack.push_back(stmt);
    i = termConds.find(callStack);
    bool firstIter = (i != termConds.end());
    fval = firstIter ? i->second : NO_VALUE;
    
    if (firstIter && otherIters) { 
        // Join first iteration value to all other iterations value
        if (val != fval) val = NO_VALUE;
    } else 
    if (firstIter && !otherIters) { 
        // If there is only one loop iteration
        val = fval;
    }
    //cout << "First iter " << fval.asString() << " " << firstIter 
    //      << ", other iters " << val.asString() << " " << otherIters << endl;
}

// Check then/else branches are empty
bool ScTraverseProc::checkIfBranchEmpty(const Stmt* branch) 
{
    if (branch) {
        if (const CompoundStmt* cmpd = dyn_cast<const CompoundStmt>(branch)) {
            return (cmpd->body_empty());
        } else 
        if (isa<const BreakStmt>(branch)) {
            return false;
        } else
        if (isa<const ContinueStmt>(branch)) {
            return false;
        }
        if (isa<const ConditionalOperator>(branch)) {
            return false;
        }
        // All other statements
        return false;
    }
    return true;
}

// Prepare next block analysis
void ScTraverseProc::prepareNextBlock(AdjBlock& nextBlock, 
                                      vector<ScopeInfo>& scopeInfos) 
{
    // Taken block must have any predecessor
    SCT_TOOL_ASSERT (!scopeInfos.empty(), "No predecessor for next block");

    // Get next scope
    shared_ptr<CodeScope> nextScope = nullptr;
    for (auto&& si : scopeInfos) {
        // Check the next scope is the same
        if (!(nextScope == si.currScope || nextScope == nullptr)) {
            cout << nextScope->asString() << " & " << si.currScope->asString() << endl;
        }
        SCT_TOOL_ASSERT (nextScope == si.currScope || nextScope == nullptr,
                         "Incorrect next scope");
        nextScope = si.currScope;
    }

    // Update predecessor scopes and return loop stack for the next block
    loopStack = getLoopStack<LoopStack>(scopeInfos);
    
    // Check for switch case block
    Stmt* lblStmt = nextBlock.getCfgBlock() ? 
                    nextBlock.getCfgBlock()->getLabel() : nullptr;
    bool switchCase = lblStmt && isa<SwitchCase>(lblStmt);
    
    unsigned currLoopLevel = (loopStack.size()) ? loopStack.back().level : 0;
    auto levelPair = calcNextLevel(scopeInfos, currLoopLevel, emptyCaseBlocks, 
                                   switchCase);
    unsigned nextLevel = levelPair.first;
    unsigned upLevel = levelPair.second;
    
    // @do part of do...while statement, increase level for loop body
    auto doterm = getDoWhileTerm(nextBlock);
    // Do not increase level for empty DoWhile as it is skipped everywhere
    if (doterm && !isDoWhileEmpty(doterm)) nextLevel += 1;
    
    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
        cout << "  scope " << nextScope->asString() << endl;
        cout << "  upLevel " << upLevel << ", has DO entry " << (doterm != nullptr) 
             <<", final level "<< nextLevel << endl;
    }
    
    // Set previous and new current block
    prevBlock = block;
    block = nextBlock;

    // Set next scope
    scopeGraph->setCurrScope(nextScope);
    scopeGraph->setCurrLevel(nextLevel);
    // Set absolute level from function and block levels
    //level = funcLevel + nextLevel; 

    // Join dead code flags by AND form all inputs 
    deadCode = true;
    deadCond = true;
    for (auto&& si : scopeInfos) {
        deadCode = deadCode && si.deadCode;
        deadCond = deadCond && si.deadCond;
    }
    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
        cout << "  dead code/dead cond " << deadCode <<"/" << deadCond << endl;
    }

    // Erase local variables in Phi functions
    //if (upLevel && !deadCode) {
    //    state->removeValuesByLevel(level);
    //}
    
    // Add next scope predecessors
    for (auto&& si : scopeInfos) {
        scopeGraph->addScopePred(si.prevScope);
    }
}

// ---------------------------------------------------------------------------
// Put counter check and decrease code for wait(n) state entrance
void ScTraverseProc::putWaitNScopeGraph(const Stmt* stmt, int waitId, 
                                        bool isResetSection)
{
    if (isWaitNStmt(stmt)) {
        auto& names = state->getWaitNVarName();
        // Use @nullptr instead of @stmt to avoid remove statement string 
        // from scope graph
        string s = "if ("+ names.first + " != 1) begin";
        scopeGraph->storeStmt(nullptr, s);
        s = codeWriter->getTabSymbol() + names.second + " = " + 
            names.first + " - 1";
        scopeGraph->storeStmt(nullptr, s);
        
        // @true add tabulation
        scopeGraph->storeStateAssign(nullptr, state->getProcStateName(),
                                     waitId, isResetSection, true,
                                     getFileName(stmt->getSourceRange().
                                     getBegin().printToString(sm)));
        scopeGraph->storeStmt(nullptr, "end");
        
        if (DebugOptions::isEnabled(DebugComponent::doGenRTL)) {
            cout << "RTL : " << (s + " ... end") << " #" << hex << stmt << dec << endl;
        }
    }
}

// ---------------------------------------------------------------------------
// Parsing functions overridden

// Register return value and prepare @lastContext
void ScTraverseProc::prepareCallContext(clang::Expr* expr, 
                                     const SValue& funcModval, 
                                     const SValue& funcRecval, 
                                     const clang::FunctionDecl* callFuncDecl,
                                     const SValue& retVal) 
{
    // Store return value for the call expression to replace where it used
    if (calledFuncs.count(expr) == 0) {
        //cout << "prepareCallContext expr " << hex << expr << dec << " retVal " << retVal << endl;
        calledFuncs.emplace(expr, retVal);
    } else {
        cout << hex << expr << dec << endl;
        SCT_TOOL_ASSERT (false, "Second meet of function call");
    }

    // Prepare context to store
    lastContext = std::make_shared<ScFuncContext>(
                                CfgCursor(funcDecl, nullptr, 0), 
                                returnValue, modval, recval, delayed, loopStack, 
                                calledFuncs, /*funcLevel,*/ deadCode, 
                                false, scopeGraph, codeWriter->serialize());

    // Set module, dynamic module and return value for called function
    modval = funcModval;
    recval = funcRecval;
    returnValue = retVal;
    funcDecl = callFuncDecl;
}

// Function call expression
void ScTraverseProc::parseCall(CallExpr* expr, SValue& val) 
{
    ScGenerateExpr::parseCall(expr, val);
    // Return value passed in @val
    SValue retVal = val;

    // Get function/method
    // Get function name and type
    FunctionDecl* funcDecl = expr->getDirectCallee();
    SCT_TOOL_ASSERT (funcDecl, "No function found for call expression");
    
    string fname = funcDecl->getNameAsString();
    auto nsname = getNamespaceAsStr(funcDecl);

    if (fname == "wait") {
        // SC wait call, wait as function presents in sc_wait.h
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "----------- wait "<< expr->getSourceRange().getBegin().printToString(sm) <<"-----------" << endl;
        }
        waitCall = true;

    } else 
    if (fname == "sct_assert") {
        // Do nothing
        
    } else 
    if (fname == "sct_assert_const" || fname == "sct_assert_unknown" || 
        fname == "__assert" || fname == "__assert_fail" || 
        fname == "sct_assert_level" || 
        (nsname && (*nsname == "sc_core" || *nsname == "sc_dt"))) {
        // Do nothing
        
    } else     
    if (fname == "sct_assert_defined" || fname == "sct_assert_register" ||
        fname == "sct_assert_read" || fname == "sct_assert_latch") {
        // Do nothing

    } else 
    if (fname == "sct_assert_in_proc_func" || 
        fname == "sct_assert_in_proc_start") {
        // Do nothing
        
    } else 
    if (((nsname && *nsname == "std") || isLinkageDecl(funcDecl)) &&
        (fname == "printf" || fname == "fprintf" || 
         fname == "sprintf" || fname == "snprintf" ||
         fname == "fopen" || fname == "fclose"))
    {
        // Do nothing 
        //cout << "ScTraverseProc::parseCall ignore function " << fname << endl;
        
    } else {
        // General function call
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for FUNCTION : " << fname << " (" 
                 << expr->getSourceRange().getBegin().printToString(sm) << ") |" << endl;
            cout << "-------------------------------------" << endl;
        }

        if (codeWriter->isParseSvaArg()) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_FUNC_IN_ASSERT);
        }
        
        // Generate function parameter assignments
        prepareCallParams(expr, modval, funcDecl);
        // Register return value and prepare @lastContext
        prepareCallContext(expr, modval, NO_VALUE, funcDecl, retVal);
        // Return value variable has call point level
        state->setValueLevel(retVal, level);
    }
}

// Member function call expression
void ScTraverseProc::parseMemberCall(CXXMemberCallExpr* expr, SValue& val) 
{
    ScGenerateExpr::parseMemberCall(expr, val);
    // Return value passed in @val
    SValue retVal = val;

    // Get method
    FunctionDecl* funcDecl = expr->getMethodDecl()->getAsFunction();
    string fname = funcDecl->getNameAsString();
    
    // Get @this expression and its type
    Expr* thisExpr = expr->getImplicitObjectArgument();
    QualType thisType = thisExpr->getType();
    bool isScCoreObject = isAnyScCoreObject(thisType);

    if (isScCoreObject && 
        (fname == "print" || fname == "dump" || fname == "kind")) {
        // Do nothing 
        //cout << "ScTraverseProc::parseMemberCall ignore function " << fname << endl;
        
    } else
    if ( isAnyScIntegerRef(thisType, true) ) {
        // Do nothing 
        
    } else 
    if ( isScChannel(thisType) ) {
        // Get value for @this
        SValue tval;
        chooseExprMethod(thisExpr, tval);
       
    } else 
    if (isScCoreObject && fname == "wait" ) {
        // SC wait call
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "----------- wait "<< expr->getSourceRange().getBegin().printToString(sm) <<"-----------" << endl;
        }
        waitCall = true;
        
    } else
    if (isScCoreObject && fname == "name" ) {
        // Do nothing for specific @sc_core functions
        
    } else {
        // General method call
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for FUNCTION : " << fname << " (" 
                 << expr->getSourceRange().getBegin().printToString(sm) << ") |" << endl;
            cout << "-------------------------------------" << endl;
        }

        if (codeWriter->isParseSvaArg()) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_FUNC_IN_ASSERT);
        }
        
        // Get value for @this
        SValue tval;
        chooseExprMethod(thisExpr, tval);
        
        // Get record from variable/dynamic object, no unknown index here
        SValue ttval = getRecordFromState(tval, ArrayUnkwnMode::amNoValue);
        //cout << "parseMemberCall tval " << tval << ", ttval " << ttval << endl;
        
        // This value *this must be a kind of record
        if (!ttval.isRecord()) {
            cout << "parseMemberCall tval " << tval << ", ttval " << ttval << endl;
            SCT_TOOL_ASSERT (false, "This expression is not record value");
        }
        
        // Call with cast this object to specific class with "::",
        // function call is not virtual in this case
        bool hasClassCast = false;
        if (auto memberExpr = dyn_cast<MemberExpr>(expr->getCallee())) {
            hasClassCast = memberExpr->hasQualifier();
        }

        // Dynamic class for member record
        SValue dyntval = ttval;
        // @modval for called function analysis
        SValue funcModval = ttval;

        // Get best virtual function and its dynamic class for @funcDecl
        if (funcDecl->isVirtualAsWritten() && !hasClassCast) {
            // Get dynamic class for member record
            state->getMostDerivedClass(ttval, dyntval);
            // Get best virtual function for dynamic class
            auto virtPair = getVirtualFunc(dyntval, funcDecl);
            funcModval = virtPair.first;
            funcDecl = virtPair.second;
        }

        // Check function is not pure
        if (funcDecl->isPure()) {
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                                 ScDiag::CPP_PURE_FUNC_CALL) << fname;
            // Pure function call leads to SIGSEGV in cfg->dump(), 
            // so do not create function call
            return;
        }

        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "Function call this class value " << ttval.asString()
                 << ", dynamic class value " << dyntval.asString() 
                 << ", funcModval " << funcModval.asString() << endl;
        }

        // Generate function parameter assignments
        prepareCallParams(expr, funcModval, funcDecl);
        // Register return value and prepare @lastContext
        prepareCallContext(expr, funcModval, NO_VALUE, funcDecl, retVal);
        // Return value variable has call point level
        state->setValueLevel(retVal, level);

        // Set record expression (record name and indices) to use in called function
        if (auto thisStr = codeWriter->getStmtString(thisExpr)) {
            codeWriter->setRecordName(funcModval, thisStr.getValue());
            //cout << "   thisStr : " << thisStr.getValue() << endl;
        }
    }
}

// Choose and run DFS step in accordance with expression type.
// Remove sub-statements from generator
void ScTraverseProc::chooseExprMethod(Stmt *stmt, SValue &val)
{
    // Constructor call for local record considered as function call
    bool anyFuncCall = isa<CallExpr>(stmt) || isa<CXXConstructExpr>(stmt);

    //cout << "--- chooseExprMethod " << hex << stmt << dec << " anyFuncCall " << anyFuncCall << endl;    
    
    // Check if no function call required, used for removing sub-statement
    // from code writer for right part of &&\|| where left part is constant
    if (anyFuncCall && noFuncCall) {
        //cout << "Function call not required, removed" << endl;
        return;
    }
    
    if (anyFuncCall && calledFuncs.count(stmt)) {
        SValue retVal = calledFuncs.at(stmt);
        //cout << "Get RET value " << retVal << " for stmt #" << hex << stmt << dec  << endl;

        // Restore return value variable as term for this expression
        codeWriter->putValueExpr(stmt, retVal);
        val = retVal;

    } else {
        // Statement for which code written cannot be part of another statement
        // Remove sub-statements for current scope and its predecessors, 
        // remove from predecessors required for logic expression with ||/&& and
        // for conditional operator
        if (!noRemoveStmt && scopeGraph) {
            scopeGraph->addSubStmt(stmt, loopStack.empty() ? 
                                   nullptr : loopStack.back().stmt);
            //cout << "Mark as sub-statement, stmt #" << hex << stmt << dec << endl;
        }
        noRemoveStmt = noRemoveStmt && (isa<CXXBindTemporaryExpr>(stmt) ||
                                        isa<CXXConstructExpr>(stmt));
        ScGenerateExpr::chooseExprMethod(stmt, val);
    }
}
// ------------------------------------------------------------------------
// Context functions

// Initialize analysis context at function entry
void ScTraverseProc::initContext() 
{
    SCT_TOOL_ASSERT (delayed.empty(), "@delayed is not empty");
    SCT_TOOL_ASSERT (loopStack.empty(), "@loopStack is not empty");
    SCT_TOOL_ASSERT (calledFuncs.empty(), "@calledFuncs is not empty");
    SCT_TOOL_ASSERT (funcDecl, "Function declaration and context stack not set");

    // Clear temporary variable index generator
    STmpVariable::clearId();
    
    auto firstScope = createScope();
    scopeGraph = std::make_shared<ScScopeGraph>(noreadyBlockAllowed);
    scopeGraph->setFirstScope(firstScope);
    scopeGraph->setCurrScope(firstScope);
    scopeGraph->setName(funcDecl->getNameAsString());
    
    // Setup first non-MIF module value
    synmodval = state->getSynthModuleValue(modval, ScState::MIF_CROSS_NUM);
    
    // Check if current module if element of array of MIF
    if (isScModularInterface(modval.getType())) {
        // @modval is array element
        bool unkwIndex;
        SValue aval = state->getBottomArrayForAny(modval, unkwIndex,
                                                  ScState::MIF_CROSS_NUM);

        if (aval.isArray()) {
            SValue mval;
            std::vector<int> indxs;
            state->getArrayIndices(aval, mval, indxs);
            
            mval = state->getVariableForValue(mval);
            string s = mval.asString(false);
            
            for (auto i : indxs) {
                SCT_TOOL_ASSERT (i >= 0, "Unknown index for MIF array element");
                s += "["+ to_string(i) +"]";
            }
            //std::cout << "setMIFName " << modval << " " << s << std::endl;
            codeWriter->setMIFName(modval, s);
        }
    }

    codeWriter->setRecordName(NO_VALUE, "");

    cfg = cfgFabric->get(funcDecl);
    block = AdjBlock(&cfg->getEntry(), true);
    prevBlock = block;
    elemIndx = 0;
    exitBlockId = cfg->getExit().getBlockID();
    deadCode = false;
    
    scopeGraph->setCurrLevel(0);
    //funcLevel = 0;
    //level = 0;
    
    state->clearValueLevels();
    sctRstAsserts.clear();
    sctAsserts.clear();
    
    if (DebugOptions::isEnabled(DebugComponent::doGenCfg)) {
        cout << "--------------------------------------" << endl;
        cout << "FUNCTION : " << funcDecl->getNameAsString() << endl;
        cfg->dump(LangOptions(), true);
        cout << "--------------------------------------" << endl;
    }    
}

// Restore analysis context with given one
// \param funcCallRestore -- restore scope graph and delayed after function call,
//                           not restored for start analysis in wait()                       
// \return -- is context stored in break/continue in removed loop analysis run
bool ScTraverseProc::restoreContext(bool funcCallRestore) 
{
    /*cout << "Context stack :" << endl;
    for (auto& i : contextStack) {
        cout << "   scopeGraph #" << hex << i.scopeGraph.get() << dec << endl;
    }*/
    
    auto context = contextStack.back();
    contextStack.pop_back();

    // Function loop stack can be not empty in return from switch/loop,
    loopStack = context.loopStack;

    modval = context.modval;
    recval = context.recval;
    returnValue = context.returnValue;
    funcDecl = context.callPoint.getFuncDecl();

    scopeGraph = context.scopeGraph;
    //funcLevel = context.funcLevel;
    //level = scopeGraph->getCurrLevel() + funcLevel;
    
    codeWriter->deserialize(context.codeWriter);

    if (funcCallRestore) {
        // Restore after function call and break/continue
        calledFuncs = context.calledFuncs;
        delayed = context.delayed;
        
        // Restore @deadCode only if context stack restored after break/continue
        // in removed loop analysis finished, after normal function call it remains
        if (context.breakContext) {
            deadCode = context.deadCode;
        }
        
    } else {
        // Restore in run after @wait() call
        // Previously called function return values cannot be used after wait()
        calledFuncs.clear();

        // Restore state, required to have local record variables 
        //state = shared_ptr<ScState>(waitStates.at(waitId).clone());
                
        // Clear delayed branches in all function context in stack
        for (auto& i : contextStack) {
            i.delayed.clear();
        }
        
        SCT_TOOL_ASSERT (delayed.empty(), "@delayed is not empty");
        // No dead code after wait here
        deadCode = false;
        
        // All loops becomes removed to avoid break/continue generating
        for (auto& i : loopStack) {
            i.removed = true;
        }
    }

    cfg = cfgFabric->get(funcDecl);
    block = AdjBlock(const_cast<CFGBlock*>(context.callPoint.getBlock()), true);
    prevBlock = block;
    // Start analysis with restored element index
    elemIndx = context.callPoint.getElementID();
    exitBlockId = cfg->getExit().getBlockID();
    
    //if (funcCallRestore && !deadCode) {
    //    // Erase local variables of called function 
    //   state->removeValuesByLevel(level);
    //}
    
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << endl;
        cout << "---------- restoreContext ------------ " << funcCallRestore << endl;
        cout << "Func " << funcDecl->getNameAsString() 
             <<  ", modval " << modval << ", deadCode " << deadCode << endl;
        //cout << "Block #" << block.getCfgBlock()->getBlockID() << " elemIndx " << elemIndx << endl;
        //cfg->dump(LangOptions(), true);
        cout << "--------------------------------------" << endl;

        //cout << " calledFuncs:" << endl;
        //for (auto& i : calledFuncs) {
        //    cout << "   stmt #" << hex << i.first << dec << endl;
        //}
        //cout << "scopeGraph #" << hex << scopeGraph.get() << ", sopce #" << scopeGraph->getFirstScope() << dec << endl;
    }
    
    return context.breakContext;
}
    
// ------------------------------------------------------------------------
// Main functions

// Preset function to run analysis, used for entry function analysis
void ScTraverseProc::setFunction(FunctionDecl* fdecl)
{
    funcDecl = fdecl;
    // Clear previous run stores wait contexts
    waitContexts.clear();
}

void ScTraverseProc::setEmptySensitivity(bool empty)
{
    emptySensitivity = empty;
    codeWriter->setEmptySensitivity(empty);
}

// Set context stack, used for thread wait-to-wait analysis
void ScTraverseProc::setContextStack(const ScProcContext& cntxStack)
{
    contextStack = cntxStack;
    // Clear previous run stores wait contexts
    waitContexts.clear();
}

// Run analysis at function entry or at @wait() call
void ScTraverseProc::run()
{
    //cout << endl << "====================== ScTraverseProc::run() ====================" << endl;
    SCT_TOOL_ASSERT (waitContexts.empty(), "Wait contexts is not empty");
    
    // Skip one element at start, required for context restore after wait() and
    // other function call, not required after break/continue in removed loop
    bool skipOneElement = false;
    // Is reset section analyzed 
    bool isResetSection = false;
    // Start at wait statement
    bool startAtWait = false;
    
    // Initialize analysis context
    if (contextStack.empty()) {
        // Start with given CFG, used for METHOD and CTHREAD function entry
        waitId = -1;
    
        // Context initialization
        initContext();

        // Check if process has reset signal
        isResetSection = !isCombProcess && hasReset;
        
    } else {
        // Start with given context, used at wait() statement in CTHREAD
        // Restore in run after @wait() call, run before @restoreContext
        waitId = cthreadStates->getStateID(contextStack.getCursorStack());
        startAtWait = true;
            
        // Restore context for wait() function call point
        restoreContext(false);
        // Skip wait()/function call
        skipOneElement = true;
    }
    
    codeWriter->setResetSection(isResetSection);
    codeWriter->clearExtrCombVarUsedRst();

    while (true)
    {
        Stmt* currStmt = nullptr;
        
        // Unreachable block can be not dead code after &&/|| condition        
        // Do not generate statement for dead code block
        if (!deadCode && !deadCond) {
            //cout << "elemId = " << elemIndx << ", block size = " << block.getCfgBlock()->size() << endl;
            SCT_TOOL_ASSERT (elemIndx <= block.getCfgBlock()->size(), 
                             "Incorrect elmId");

            // Generate @do part of @do...@while statement
            auto doterm = getDoWhileTerm(block);
            // Skip @do after context restore, skip empty DoWhile
            if (doterm && !skipOneElement && !isDoWhileEmpty(doterm)) {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    cout << "DO..WHILE entry, generate @do" << endl;
                }
                
                // Put @if instead of @while in thread process
                bool ifStmt = findWaitInLoop && 
                              findWaitInLoop->hasWaitCall(doterm);

                // Store @while part of do...while statement in code writer
                if (!ifStmt) {
                    // Get terminator condition value from CPA
                    SValue termCValue; SValue termCValueFirst;
                    getTermCondValue(doterm, termCValue, termCValueFirst);

                    auto stmtStr = parseTerm(doterm, termCValue,  false);
                    scopeGraph->storeStmt(doterm, stmtStr.getValue());
                }

                // Add current loop into loop stack
                loopStack.pushLoop({doterm, scopeGraph->getCurrLevel()-1, ifStmt});
            }

            // CFG block body analysis, preformed for not dead state only
            for (size_t i = elemIndx; i < block.getCfgBlock()->size(); ++i)
            {
                const CFGElement& elm = block.getCfgBlock()->operator [](i);
                if (elm.getKind() == CFGElement::Kind::Statement) {
                    // Get statement 
                    CFGStmt* s = elm.getAs<CFGStmt>().getPointer();
                    currStmt = const_cast<Stmt*>(s->getStmt());
                    
                    if (DebugOptions::isEnabled(DebugComponent::doGenStmt)) {
                        cout << endl;
                        currStmt->dumpColor();
                        //state->print();
                        //cout << " getCurrLevel " << scopeGraph->getCurrLevel() << endl;
                    }
                    
                    // Put @wait(int n) counter check and decrement
                    if (startAtWait) {
                        putWaitNScopeGraph(currStmt, waitId, isResetSection);
                        startAtWait = false;
                    }
                    
                    // If started with restored context, move to the next element, 
                    // element stored in context was already analyzed
                    if (skipOneElement) {
                        if (DebugOptions::isEnabled(DebugComponent::doGenStmt)) {
                            cout << "SKIP this statement after restore context" << endl;
                        }
                        skipOneElement = false;
                        continue;
                    }

                    // Parse statement expression and generate RTL
                    // Do not remove sub-statement of CXXBindTemporaryExpr
                    // as it not stored in scope graph (to avoid unused return)
                    noRemoveStmt = true; //isa<CXXBindTemporaryExpr>(currStmt);
                    auto stmtStr = parse(currStmt);
                    //cout << "Stmt " << hex << currStmt << dec << " children " << endl;
                    //for (auto c : currStmt->children()) {
                    //    cout << hex << c << dec << endl;
                    //}
                    noRemoveStmt = false;
                    
                    // Store statement if it has term only, that ignores 
                    // declaration statements, also check for empty statement
                    // for example function call w/o result assignment
                    if (stmtStr && !isa<CXXBindTemporaryExpr>(currStmt)) {
                        if (isTemporalAssert(currStmt)) {
                            // Temporal assertion in process
                            if (!inMainLoop) {
                                if (!noSvaGenerate) {
                                    if (auto str = getSvaInLoopStr(*stmtStr, 
                                                   isResetSection)) {
                                        if (isResetSection) {
                                            sctRstAsserts.insert(*str);
                                        } else {
                                            sctAsserts.insert(*str);
                                        }
                                    }
                                }
                            } else {
                                ScDiag::reportScDiag(currStmt->getBeginLoc(),
                                                ScDiag::SYNTH_SVA_IN_MAIN_LOOP);
                            }

                        } else {
                            // Normal statement, store it in scope graph
                            scopeGraph->storeStmt(currStmt, *stmtStr);
                            if (emptySensitivity) {
                                scopeGraph->setEmptySensStmt(currStmt);
                            }
                        }
                        
                        if (DebugOptions::isEnabled(DebugComponent::doGenRTL)) {
                            cout << "RTL : " << *stmtStr << " #" << hex << currStmt << dec << endl;
                        }
                    }
                    
                    // Wait call, stop this block analysis
                    if (waitCall && cthreadStates) {
                        // @calledFuncs really not used in context restore
                        auto waitCntx = ScFuncContext(
                                CfgCursor(funcDecl, block.getCfgBlock(), i), 
                                returnValue, modval, recval, delayed, loopStack, 
                                calledFuncs, /*funcLevel,*/ false,false, scopeGraph,
                                codeWriter->serialize());
                        
                        /*cout << " waitCall #" << hex << scopeGraph.get() << dec 
                             << " getCurrLevel " << scopeGraph->getCurrLevel()
                             << " funcLevel " << funcLevel << endl;*/
                        
                        // Add current call statement to the context, use clone
                        // to allocate scope graph pointer to avoid changing it
                        contextStack.push_back(waitCntx);
                        auto waitCntxStack = contextStack.clone();
                        waitContexts.push_back(waitCntxStack);
                        contextStack.pop_back();
                        
                        /*cout << "getCursorStack " << endl;
                        for (auto& i : waitCntxStack.getCursorStack()) {
                                cout << "   " << i.getFuncDecl()->getNameAsString() 
                                     << " B#" << i.getBlock()->getBlockID() 
                                     << " elm#" << i.getElementID() << endl;
                        }
                        cout << "----------------" << endl;*/
                        
                        // Generate state variable assignment in scope graph
                        waitId = cthreadStates->getStateID(
                                            waitCntxStack.getCursorStack());
                        
                        // Insert or join current state for this @wait()
                        /*auto i = waitStates.find(waitId);
                        if (i == waitStates.end()) {
                            waitStates.emplace(waitId, *(state.get()));
                        } else {
                            i->second.join(state.get());
                        }*/
                        
                        //assert (!stmtStr && "Extra string for wait() call");
                        //cout << "WAIT stmtStr " << ((stmtStr) ? stmtStr.getValue() : "") << endl;
                        
                        if (!isSingleStateThread) {
                            // @false no tabulation added
                            scopeGraph->storeStateAssign(
                                    currStmt, state->getProcStateName(),
                                    waitId, isResetSection, false,
                                    getFileName(currStmt->getSourceRange().
                                    getBegin().printToString(sm)));
                        }
                        
                        // Continue analysis with dead code set
                        waitCall = false;
                        deadCode = true;
                        break;
                        
                    } else 
                    if (waitCall && !cthreadStates) {
                        ScDiag::reportScDiag(currStmt->getSourceRange().getBegin(), 
                                             ScDiag::SC_WAIT_IN_METHOD);
                    }
                    
                    // Run analysis of called function in this traverse process
                    if (lastContext) {
                        // Add empty string, case if function call has no parameters,
                        // required to have statement for function call in the scope
                        if (!stmtStr) {
                            scopeGraph->storeStmt(currStmt, string());
                        }

                        // Set block and element into context
                        lastContext->callPoint = CfgCursor(
                                lastContext->callPoint.getFuncDecl(),
                                block.getCfgBlock(), i);

                        // Get(create) CFG for function and setup next block
                        cfg = cfgFabric->get(funcDecl);
                        if (!cfg) {
                            SCT_INTERNAL_ERROR(currStmt->getBeginLoc(), 
                                               "Function CFG is null");
                        }
                        block = AdjBlock(&cfg->getEntry(), true);
                        prevBlock = block;
                        exitBlockId = cfg->getExit().getBlockID();
                        
                        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                            cout << "--------------------------------------" << endl;
                            cfg->dump(LangOptions(), true);
                            cout << "--------------------------------------" << endl;
                        }
                        
                        // Create new scope graph
                        scopeGraph = std::make_shared<ScScopeGraph>(
                                                        noreadyBlockAllowed);
                        auto firstScope = createScope();
                        scopeGraph->setFirstScope(firstScope);
                        scopeGraph->setCurrScope(firstScope);
                        // Set function name
                        scopeGraph->setName(funcDecl->getNameAsString());

                        // Set level for called function
                        scopeGraph->setCurrLevel(0);
                        //funcLevel = funcLevel + level+1;
                        
                        // Store scope graph in callee scope graph,
                        // used to print called function body statements
                        lastContext->scopeGraph->addFCallScope (
                                    currStmt, loopStack.empty() ? nullptr : 
                                    loopStack.back().stmt, scopeGraph);
                        
                        // Store and null function context
                        contextStack.push_back( *(lastContext.get()) );
                        
                        // Start called function with empty @delayed and loop stack
                        delayed.clear();
                        loopStack.clear();
                        calledFuncs.clear();
                        
                        lastContext = nullptr;
                        funcCall = true;
                        break;
                    }
                } else {
                    SCT_INTERNAL_FATAL_NOLOC ("Incorrect element kind");
                }
            }
        } else {
            if (DebugOptions::isEnabled(DebugComponent::doGenStmt)) {
                cout << "No statements generated for block with dead code/dead cond" << endl;
            }
        }

        // Start any block expect return to callee with first element
        elemIndx = 0;
        // No skip element in next block required (possible after Phi function)
        skipOneElement = false;
        
        // Block successors
        vector<pair<AdjBlock, ScopeInfo> >  blockSuccs;
        // Block terminator
        auto cfgBlock = block.getCfgBlock();
        SCT_TOOL_ASSERT(cfgBlock, "Current CFG block is null");
        const Stmt* term = cfgBlock->getTerminator().getStmt();
        
        bool breakInRemovedLoop = false;
        bool contInRemovedLoop = false;
        
        if (funcCall) {
            // Suspend analysis of this function and go to to called function
            funcCall = false;
            continue;
            
        } else 
        if (term) {
            // Erase value at loop entry level, therefore "-1" for DO..WHILE
            //bool doWhile = isa<const DoStmt>(term);
            //if (!deadCode) {
            //    state->removeValuesByLevel(term == mainLoopTerm ? 0 : 
            //                               doWhile ? level-1 : level);
            //}

            // Get terminator condition value for all/first iteration from CPA
            SValue termCValue; SValue termCValueFirst;
            getTermCondValue(term, termCValue, termCValueFirst);
            
            if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                cout << "termCValue " << termCValue.asString() << endl;
            }
            //cout << "termCValue term# " << hex << term << dec << " val " << termCValue.asString() << endl;
            //term->dumpColor();
            
            if (const IfStmt* ifstmt = dyn_cast<const IfStmt>(term)) {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    if (!deadCode) term->dumpColor();
                }
                
                bool deadThen = termCValue.isInteger() && 
                                termCValue.getInteger().isNullValue();
                bool deadElse = termCValue.isInteger() && 
                                !termCValue.getInteger().isNullValue();
                
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2, 
                                 "No two successors in IF");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                // Then block always exists even if it is empty
                AdjBlock thenBlock(*iter);
                // If Else branch is empty, Else block is next IF block
                AdjBlock elseBlock(*(++iter));
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "IF ThenBlock B" << thenBlock.getCfgBlockID() << ((thenBlock.isReachable()) ? "" : " is unreachable") << endl;
                    cout << "IF ElseBlock B" << elseBlock.getCfgBlockID() << ((elseBlock.isReachable()) ? "" : " is unreachable") << endl;
                }
                // Create new/reuse scopes for the branches, 
                // blocks for Then and Else are always different
                SCT_TOOL_ASSERT (thenBlock != elseBlock,
                                 "The same then and else blocks in IF");
                auto thenScope = getScopeForBlock(thenBlock);
                auto elseScope = getScopeForBlock(elseBlock);
                // If Else branch is empty, Else block is next IF block
                bool elseIsEmpty = checkIfBranchEmpty(ifstmt->getElse()); 

                // Set Then/Else scopes as successors of current IF scope
                scopeGraph->addScopeSucc(thenScope);
                // For empty Else create empty scope and connect it
                if (elseIsEmpty) {
                    auto emptyScope = createScope();
                    scopeGraph->setScopeLevel(emptyScope, 
                                              scopeGraph->getCurrLevel()+1);
                    scopeGraph->addScopeSucc(emptyScope);
                    scopeGraph->addScopePred(emptyScope, 
                                             scopeGraph->getCurrScope());
                    scopeGraph->addScopeSucc(emptyScope, elseScope);
                    
                } else {
                    scopeGraph->addScopeSucc(elseScope);
                }

                // Check if the next block is loop input (has loop terminator) 
                // and current block is loop body input/outside input
                bool thenLoopInputBody = checkLoopInputFromBody(block, thenBlock);
                bool thenLoopInputOut  = checkLoopInputFromOut(block, thenBlock);
                bool elseLoopInputBody = checkLoopInputFromBody(block, elseBlock);
                bool elseLoopInputOut  = checkLoopInputFromOut(block, elseBlock);
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "   thenBlock flags " << thenLoopInputBody << ", " << thenLoopInputOut << endl;
                    cout << "   elseBlock flags " << elseLoopInputBody << ", " << elseLoopInputOut << (elseIsEmpty ? " is EMPTY" : "") << endl;
                    cout << "IF dead " << deadCode << " then/else " <<(deadCode || deadThen) << (deadCode || deadElse) << endl;
                }    
                // Normal non-empty branch block has +1 level
                ScopeInfo thenES(scopeGraph->getCurrLevel()+1, block,
                                 scopeGraph->getCurrScope(), thenScope,
                                 thenLoopInputBody, thenLoopInputOut, 
                                 loopStack, deadCode || deadThen);
                // If Else branch is empty, next block has level up even if 
                // there is no @Phi function, so set level up flag
                ScopeInfo elseES(scopeGraph->getCurrLevel()+1, block,
                                 scopeGraph->getCurrScope(), elseScope,
                                 elseLoopInputBody, elseLoopInputOut, 
                                 loopStack, deadCode || deadElse, nullptr, 
                                 elseIsEmpty);
                // Store ready to analysis block last, to get it first
                blockSuccs.push_back({elseBlock, elseES});
                blockSuccs.push_back({thenBlock, thenES});

                // Parse IF statement, remove sub-statements in code writer
                if (!deadCode) {
                    auto stmtStr = parseTerm(term, termCValue);
                    if (!emptySensitivity) {
                        scopeGraph->storeStmt(term, stmtStr.getValue());
                    }
                }

            } else
            if (const SwitchStmt* swstmt = dyn_cast<const SwitchStmt>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    if (!deadCode) term->dumpColor();
                }

                // Add current switch into loop stack
                loopStack.pushLoop({term, scopeGraph->getCurrLevel(), false});
                
                // Switch cases, skip first case as it belongs to @default
                const SwitchCase* swcase = swstmt->getSwitchCaseList();
                bool hasDefault = false;
                bool emptyDefault = false;
                if (swcase && isa<const DefaultStmt>(swcase) ) {
                    hasDefault = true;
                    emptyDefault = isDefaultEmpty(swcase);
                    swcase = swcase->getNextSwitchCase();
                }
                
                // Cases with their blocks, reordered in direct order
                auto cases = getSwitchCaseBlocks(swcase, cfgBlock);
                
                /*cout << "cases : " << endl;
                for (auto entry : cases) {
                    cout << "   " << hex << entry.first << dec << " block #" << entry.second->getBlockID() << endl;
                    entry.first->dumpColor();
                }*/
                
                // Cases loop
                bool constCase = false; // One case chosen by constant condition
                for (auto entry : cases) {
                    const SwitchCase* swcase = entry.first;
                    AdjBlock caseBlock = entry.second;
                    SCT_TOOL_ASSERT (caseBlock.getCfgBlock(), "No case block");
                    bool emptyCase = isCaseEmpty(caseBlock);
                    if (emptyCase) emptyCaseBlocks.insert(caseBlock);
                   
                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << "CaseBlock B" << caseBlock.getCfgBlockID() 
                             << ((caseBlock.isReachable()) ? "" : 
                                 " is unreachable") << endl;
                        if (!caseBlock.isReachable()) {
                            cout << "Unreachable block at" << term->
                                    getBeginLoc().printToString(sm) << endl;
                        }
                    }
                    
                    // Create new/reuse scopes for case branch
                    auto caseScope = getScopeForBlock(caseBlock);
                    scopeGraph->addScopeSucc(caseScope);
                    
                    SValue caseValue;
                    if (auto* cstmt = dyn_cast<const CaseStmt>(swcase)) {
                        // Get case expression and store it in generator
                        Expr* expr = const_cast<Expr*>(cstmt->getLHS());
                        // Evaluate case value
                        evaluateConstInt(expr, caseValue);
                        // Store case expression value
                        if (auto caseStr = parse(expr)) {
                            scopeGraph->storeCase(caseScope, caseStr.getValue(),
                                                  emptyCase);
                        } else {
                            scopeGraph->storeCase(caseScope, "default", false);
                        }    
                        
                    } else {
                        SCT_TOOL_ASSERT (false, 
                                         "Unexpected statement type in switch");
                    }

                    bool loopInputBody = checkLoopInputFromBody(block, caseBlock);
                    bool loopInputOut = checkLoopInputFromOut(block, caseBlock);

                    bool deadCase = false;
                    if (termCValue.isInteger() && caseValue.isInteger()) {
                        deadCase = !APSInt::isSameValue(termCValue.getInteger(), 
                                                        caseValue.getInteger());
                        constCase = constCase || !deadCase;
                    }
                    
                    // Case/default have +1 level, loop stack with this @switch
                    ScopeInfo caseSI(scopeGraph->getCurrLevel()+1, block, 
                                     scopeGraph->getCurrScope(), caseScope,
                                     loopInputBody, loopInputOut, loopStack,
                                     deadCode || deadCase);
                    blockSuccs.push_back({caseBlock, caseSI});
                    
                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << "    block level " << caseSI.level << ", flags " << loopInputBody << ", " << loopInputOut << endl;
                    }
                }

                // Last successor block is default or after switch block
                AdjBlock succBlock(*cfgBlock->succ_rbegin());
                
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Switch Default/Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                    if (!succBlock.isReachable()) {
                        cout << "Unreachable block at" << term->getSourceRange().getBegin().printToString(sm) << endl;
                    }
                }
                
                auto succScope = getScopeForBlock(succBlock);
                scopeGraph->addScopeSucc(succScope);
                
                // Write switch code only if there are several branches
                bool writeSwitch = (cfgBlock->succ_size() > 1);
                
                unsigned level; 
                const Stmt* loopExit;
                
                if (hasDefault) {
                    // Default branch
                    scopeGraph->storeCase(succScope, "default", false);
                    // Default scope level +1 if there are case scopes
                    level = scopeGraph->getCurrLevel() + 
                            ((writeSwitch && !emptyDefault) ? 1 : 0);
                    loopExit = emptyDefault ? term : nullptr;
                    if (emptyDefault) emptyCaseBlocks.insert(succBlock);
                    
                } else {
                    // No default scope, add next block successor
                    level = scopeGraph->getCurrLevel();
                    loopExit = term;
                }
                
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);
                
                // Set loop exit to avoid level up in taking @switch next block
                bool deadDefault = hasDefault && 
                                   (constCase || !succBlock.isReachable());
                ScopeInfo switchSI(level,  block,
                                   scopeGraph->getCurrScope(), succScope,
                                   loopInputBody, loopInputOut, loopStack, 
                                   deadCode || deadDefault, loopExit);
                blockSuccs.push_back({succBlock, switchSI});
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "    block level " << switchSI.level << ", flags " 
                         << loopInputBody << ", " << loopInputOut << endl;
                }

                // Parse switch statement, remove sub-statements in code writer
                if (!deadCode) {
                    auto stmtStr = parseTerm(term, termCValue);
                    if (writeSwitch) {
                        scopeGraph->storeStmt(term, stmtStr.getValue());
                    }
                }

            } else 
            if (const BinaryOperator* binstmt = dyn_cast<const BinaryOperator>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    if (!deadCode) term->dumpColor();
                }

                BinaryOperatorKind opcode = binstmt->getOpcode();
                SCT_TOOL_ASSERT (opcode == BO_LOr || opcode == BO_LAnd,
                                 "Incorrect terminator statement");

                // For && Then branch goes to right part of && / final IF, 
                // Else branch can go to right part of && or else/exit block
                // For || Then branch can go to right part of && or else/exit block, 
                // Else branch goes to right part of && / final IF
                bool deadThen = opcode == BO_LAnd && termCValue.isInteger() &&  
                                    termCValue.getInteger().isNullValue();
                bool deadElse = opcode == BO_LOr && termCValue.isInteger() && 
                                    !termCValue.getInteger().isNullValue();
                
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in ||/&&");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock thenBlock(*iter);
                AdjBlock elseBlock(*(++iter));
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "&&/|| ThenBlock B" << thenBlock.getCfgBlockID() << ((thenBlock.isReachable()) ? "" : " is unreachable") << endl;
                    cout << "&&/|| ElseBlock B" << elseBlock.getCfgBlockID() << ((elseBlock.isReachable()) ? "" : " is unreachable") << endl;
                    cout << "&&/|| dead code " << deadCode << " dead cond " << (deadCode || deadThen) << (deadCode || deadElse) << endl;
                }
                // Check if the next block is loop input (has loop terminator) 
                // and current block is loop body input/outside input
                bool thenLoopInputBody = checkLoopInputFromBody(block, thenBlock);
                bool thenLoopInputOut  = checkLoopInputFromOut(block, thenBlock);
                bool elseLoopInputBody = checkLoopInputFromBody(block, elseBlock);
                bool elseLoopInputOut  = checkLoopInputFromOut(block, elseBlock);
                
                // Add only scope towards final IF/Loop/? as body block could
                // be removed if final terminator condition is false
                // @deadThen/Else not consider @deadCond required for complex conditions
                if (opcode == BO_LOr) {
                    auto elseScope = getScopeForBlock(elseBlock);
                    ScopeInfo elseSI(scopeGraph->getCurrLevel(), block, 
                                     scopeGraph->getCurrScope(), elseScope,
                                     elseLoopInputBody, elseLoopInputOut, loopStack,
                                     deadCode, nullptr, false, deadElse);
                    blockSuccs.push_back({elseBlock, elseSI});
                    scopeGraph->addScopeSucc(elseScope);

                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << "&&/|| elseBlock level " << elseSI.level << ", flags " << elseLoopInputBody << ", " << elseLoopInputOut << " scope " << (elseScope ? elseScope->asString() : "")<< endl;
                    }
                }
                
                if (opcode == BO_LAnd) {
                    auto thenScope = getScopeForBlock(thenBlock);
                    ScopeInfo thenSI(scopeGraph->getCurrLevel(), block,
                                     scopeGraph->getCurrScope(), thenScope,
                                     thenLoopInputBody, thenLoopInputOut, loopStack,
                                     deadCode, nullptr, false, deadThen);
                    blockSuccs.push_back({thenBlock, thenSI});
                    scopeGraph->addScopeSucc(thenScope);

                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << "&&/|| thenBlock level " << thenSI.level << ", flags " << thenLoopInputBody << ", " << thenLoopInputOut << " scope " << (thenScope ? thenScope->asString() : "")<< endl;
                    }
                }

                // Parse &&/|| condition to remove sub-statements from code writer
                if (!deadCode) {
                    parseTerm(term, termCValue);
                    // No code generated
                }
                
            } else
            if ( isa<const ConditionalOperator>(term) )
            {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    if (!deadCode) term->dumpColor();
                }

                bool deadThen = termCValue.isInteger() && 
                                termCValue.getInteger().isNullValue();
                bool deadElse = termCValue.isInteger() && 
                                !termCValue.getInteger().isNullValue();
                
                // Two argument blocks    
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in cond operator");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock argBlock1(*iter);
                AdjBlock argBlock2(*(++iter));
                // Block in conditional operator can be unreachable
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    //cout << "? ArgBlock1 B" << argBlock1.getCfgBlockID() << ((argBlock1.isReachable()) ? "" : " is unreachable") << endl;
                    //cout << "? ArgBlock2 B" << argBlock2.getCfgBlockID() << ((argBlock2.isReachable()) ? "" : " is unreachable") << endl;
                    //cout << "? dead " << deadCode << (deadCode || deadThen) << (deadCode || deadElse) << endl;
                }
                // Create new/reuse scopes for the branches
                auto argScope1 = getScopeForBlock(argBlock1);
                auto argScope2 = getScopeForBlock(argBlock2);
                scopeGraph->addScopeSucc(argScope1);
                scopeGraph->addScopeSucc(argScope2);
                
                // Arguments have +1 level to return the same level for ? body 
                // No loop terminator possible in argument blocks
                ScopeInfo argSI1(scopeGraph->getCurrLevel()+1, block,
                                 scopeGraph->getCurrScope(), argScope1,
                                 false, false, loopStack, deadCode || deadThen);
                ScopeInfo argSI2(scopeGraph->getCurrLevel()+1, block, 
                                 scopeGraph->getCurrScope(), argScope2,
                                 false, false, loopStack, deadCode || deadElse);
                blockSuccs.push_back({argBlock1, argSI1});
                blockSuccs.push_back({argBlock2, argSI2});
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << " block1 level " << argSI1.level << endl;
                    cout << " block2 level " << argSI2.level << endl;
                }
                
                // Parse ? condition to remove sub-statements from code writer
                if (!deadCode) {
                    parseTerm(term, termCValue);
                    // No code generated
                }

            } else                 
            if (isa<const ForStmt>(term) || isa<const WhileStmt>(term) ||
                isa<const DoStmt>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    if (!deadCode) term->dumpColor();
                }

                // Normally one iteration of loop is passed
                auto doWhile = dyn_cast<const DoStmt>(term);
                // Determine loop already visited with checking if 
                // current (last) loop has the same terminator
                bool loopVisited = doWhile || loopStack.isCurrLoop(term);
                // DoWhile with false condition
                bool skipDoWhile = doWhile && termCValue.isInteger() && 
                                   termCValue.getInteger().isNullValue();
                // DoWhile has empty body
                bool emptyDoWhile = doWhile && isDoWhileEmpty(doWhile);
                
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in loop");
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                // Go to loop body or to loop successor
                AdjBlock succBlock;
                AdjBlock bodyBlock(*iter);
                AdjBlock exitBlock(*(++iter));

                // Determine loop generation mode
                bool noLoopStmt = false;
                bool skipLoop = false;
                bool ifStmt = false;

                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    cout << "Loop terminator, level " << scopeGraph->getCurrLevel() << endl;
                }
    
                if (findWaitInLoop && findWaitInLoop->hasWaitCall(term)) {
                    // Loop with wait in thread 
                    SCT_TOOL_ASSERT (!isCombProcess, 
                                     "Wait in combinational process");
                    
                    // Loop condition value at current iteration (first or others)
                    SValue iterCValue = (loopVisited) ? 
                                        termCValue : termCValueFirst;
                    
                    noLoopStmt = !loopVisited && iterCValue.isInteger() && 
                                 !iterCValue.getInteger().isNullValue() && 
                                 !deadCode;
                    skipLoop = (!loopVisited && iterCValue.isInteger() && 
                                 iterCValue.getInteger().isNullValue()) || 
                                 deadCode;
                    // Check @skipDoWhile required for do/while with wait()
                    ifStmt = (loopVisited || iterCValue.isUnknown()) && 
                             !deadCode && !skipDoWhile;

                    // Set enter into main loop, used to check SVA generation
                    if (!deadCode && termCValue.isInteger() && 
                        !termCValue.getInteger().isNullValue()) {
                        inMainLoop = true;
                    }
                    
                } else {
                    // Loop without wait in thread or method
                    // One condition for all iterations
                    skipLoop = (!loopVisited && termCValue.isInteger() && 
                                 termCValue.getInteger().isNullValue()) || 
                                 deadCode;
                }
                if (DebugOptions::isEnabled(DebugComponent::doGenLoop)) {
                    cout << "LOOP : noLoopStmt " << noLoopStmt << ", skipLoop " << skipLoop 
                         << ", ifStmt " << ifStmt << ", exitBlock " << exitBlock.getCfgBlock() 
                        << ", loopVisited " << loopVisited << endl;
                }
                
                shared_ptr<CodeScope> nextScope = nullptr;
                shared_ptr<CodeScope> elseScope = nullptr;
                unsigned nextLevel;
                const Stmt* loopExit = nullptr;

                if (ifStmt) {
                    // Create IF statement instead of loop
                    SCT_TOOL_ASSERT (bodyBlock.getCfgBlock(), 
                                     "Loop body block in NULL");
                    succBlock = bodyBlock;
                    //nextScope = createScope();
                    nextScope = getScopeForBlock(succBlock);

                    // Add current loop into loop stack
                    loopStack.pushLoop({term, scopeGraph->getCurrLevel(), true});
                    
                    // Get CFGBlock* for @exitBlock here
                    if (exitBlock.getCfgBlock()) {
                        // Parse loop as IF statement 
                        auto stmtStr = parseTerm(term, termCValue, true);
                        
                        // Store IF statement in code writer
                        scopeGraph->storeStmt(term, stmtStr.getValue(), true);
                        
                        // Get scope for exit block, it cannot be achieved 
                        // from any path through THEN(loop body) block
                        auto exitScope = getScopeForBlock(exitBlock);
                        
                        // Prepare else scope empty
                        elseScope = createScope();
                        scopeGraph->setScopeLevel(elseScope, 
                                                  scopeGraph->getCurrLevel()+1);
                        scopeGraph->addScopePred(elseScope, 
                                                 scopeGraph->getCurrScope());
                        scopeGraph->addScopeSucc(elseScope, exitScope);

                        // Check if the next block is loop input Phi function and 
                        // current block is loop body input/outside input
                        bool loopInputBody = checkLoopInputFromBody(
                                                        block, exitBlock);
                        bool loopInputOut  = checkLoopInputFromOut(
                                                        block, exitBlock);

                        // Exit scope has the same level, use updated loop stack
                        // and loop exit here to remove this loop at exit
                        ScopeInfo si(scopeGraph->getCurrLevel(), block, 
                                     scopeGraph->getCurrScope(), 
                                     exitScope, loopInputBody, loopInputOut, 
                                     loopStack, deadCode, term);
                        // Exit block will be analyzed after body because of level
                        blockSuccs.push_back({exitBlock, si});
                        
                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "Exit block B" << exitBlock.getCfgBlockID() 
                                 << " scope " << exitScope->asString() << ", level " 
                                 << scopeGraph->getCurrLevel() << endl;
                        }

                        // Normal IF branch block has +1 level
                        nextLevel = scopeGraph->getCurrLevel()+1;
                        
                        // Remove already visited For loop @init pre-statement
                        if (loopVisited) {
                            if (auto forstmt = dyn_cast<const ForStmt>(term)) {
                                auto init = forstmt->getInit();
                                if (init != nullptr) {
                                    scopeGraph->removeStmt(init, 
                                                    loopStack.empty() ? nullptr: 
                                                    loopStack.back().stmt
                                                );
                                }
                            }
                        }
                        
                    } else {
                        // No IF generated, level not changed
                        // This required for main infinite loop
                        nextLevel = scopeGraph->getCurrLevel();
                    }
                    
                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << "Body block B" << succBlock.getCfgBlockID() 
                             << " scope " << nextScope->asString() << endl;
                    }

                    // Remove loop condition pre-statement
                    auto cond = getTermCond(term);
                    if (cond) {
                        // cout << "Remove 1 stmt #" << hex << cond << dec << endl;
                        scopeGraph->removeStmt(cond, loopStack.empty() ? 
                                               nullptr : loopStack.back().stmt);
                    }
                    
                } else 
                // Body block may be NULL for loop with false condition
                if (!loopVisited && bodyBlock.getCfgBlock() && !skipLoop) {
                    // Enter loop body
                    succBlock = bodyBlock;
                    //nextScope = createScope();
                    nextScope = getScopeForBlock(succBlock);

                    // Add current loop into loop stack
                    loopStack.pushLoop({term, scopeGraph->getCurrLevel(), 
                                        noLoopStmt});

                    if (!noLoopStmt) {
                        // Loop without wait()
                        // Parse loop with removing sub-statements
                        auto stmtStr = parseTerm(term, termCValue, false);
                        // Store loop statement in code writer
                        scopeGraph->storeStmt(term, stmtStr.getValue());

                        // Loop body has +1 level
                        nextLevel = scopeGraph->getCurrLevel()+1;
                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "Loop Body Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                        }
                        
                    } else {
                        // No loop generated, level not changed
                        nextLevel = scopeGraph->getCurrLevel();
                        // Set main loop level to one
                        /*if (level == 0 && !mainLoopTerm) {
                            mainLoopTerm = const_cast<Stmt*>(term);
                            level = 1; funcLevel = 1;
                            //cout << "SET MAIN LOOP ENTRY LEVEL 1" << endl;
                        }*/
                       
                        // Remove loop condition pre-statement
                        auto cond = getTermCond(term);
                        if (cond) {
                            // cout << "Remove 2 stmt #" << hex << cond << dec << endl;
                            scopeGraph->removeStmt(cond, loopStack.empty() ? 
                                            nullptr : loopStack.back().stmt);
                        }                
                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "No loop statement generated Block B"<< succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                        }
                    }

                } else 
                // If body block is NULL or loop is skipped, go to exit block
                if (loopVisited || !bodyBlock.getCfgBlock() || skipLoop) {
                    // Check @deadCode if come from loop body @loopVisited
                    // to avoid duplicating loop exit block predecessor
                    if (exitBlock.getCfgBlock() && !(deadCode && loopVisited)) {
                        // Exit from the loop, can be already reached by break exit 
                        succBlock = exitBlock;
                        nextScope = getScopeForBlock(succBlock);
                        
                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "Loop Exit Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                        }    
                        // If no loop body enter, add current loop into loop stack
                        if (!bodyBlock.getCfgBlock() || skipLoop || emptyDoWhile) {
                            loopStack.pushLoop(
                                    {term, scopeGraph->getCurrLevel(), true});
                        }

                        // Successor block can contain several inputs from 
                        // this loop and another block(s)
                        loopExit = term;

                        // Loop successor block has same level as the loop block
                        SCT_TOOL_ASSERT (!loopStack.empty(), 
                                         "Empty loop stack at loop exit");
                        auto& currLoop = loopStack.back();
                        SCT_TOOL_ASSERT (currLoop.stmt == term, 
                                         "Incorrect current loop");
                        nextLevel = currLoop.level;
                    }
                    
                    // Remove For loop @init and @incr pre-statements
                    if (!deadCode) {
                        if (auto forstmt = dyn_cast<const ForStmt>(term)) {
                            auto init = forstmt->getInit();
                            auto incr = forstmt->getInc();
                            // Remove initialization only for visited loop
                            if (loopVisited) {
                                if (init != nullptr) {
                                    scopeGraph->removeStmt(init, 
                                                    loopStack.empty() ? nullptr: 
                                                    loopStack.back().stmt
                                                );
                                }
                            }    
                            if (incr != nullptr) {
                                scopeGraph->removeStmt(incr, 
                                                    loopStack.empty() ? nullptr: 
                                                    loopStack.back().stmt
                                                );
                            }
                        }
                        // Remove loop condition sub-statements which required 
                        // for condition with binary operator ||/&&
                        // and remove condition itself
                        if (auto cond = getTermCond(term)) {
                            SValue val;
                            chooseExprMethod(const_cast<Expr*>(cond), val);
                            // cout << "Remove 3 stmt #" << hex << cond << dec << endl;
                            scopeGraph->removeStmt(cond, 
                                                   loopStack.empty() ? nullptr : 
                                                   loopStack.back().stmt
                                                );
                        }
                    }
                }

                // If there is successor block, add it to next analysis
                if (succBlock.getCfgBlock()) {
                    // Set @nextScope as successor of current scope
                    scopeGraph->addScopeSucc(nextScope);

                    // Check if the next block is loop input Phi function and 
                    // current block is loop body input/outside input
                    bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                    bool loopInputOut  = checkLoopInputFromOut(block, succBlock);

                    // Next block, loop body or successor block
                    ScopeInfo si(nextLevel, block, scopeGraph->getCurrScope(), 
                                 nextScope, loopInputBody, loopInputOut, 
                                 loopStack, deadCode, loopExit);
                    blockSuccs.push_back({succBlock, si});
                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << "    block level "<< si.level <<", flags " << loopInputBody << ", " << loopInputOut 
                             << " scope then/else " << (nextScope ? nextScope->asString() : "") << "/" << (elseScope ? elseScope->asString() : "") << endl;
                    }
                }
                
                // Add IF statement else scope after then scope
                if (elseScope.get()) {
                    scopeGraph->addScopeSucc(elseScope);
                }
                
            } else
            if ( isa<const BreakStmt>(term) ) {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    if (!deadCode) term->dumpColor();
                }
                SCT_TOOL_ASSERT (!loopStack.empty(), 
                                 "Loop stack is empty in break");
                // Parse break statement, @termCondValue not used
                auto stmtStr = parseTerm(term, termCValue);
    
                // Generate @break statement string
                auto& currLoop = loopStack.back();
                if (!isa<const SwitchStmt>(currLoop.stmt) && !deadCode) {
                    // Do not write @break for removed loop and switch statement
                    if (!currLoop.removed) {
                        scopeGraph->storeStmt(term, stmtStr.getValue());
                    } else {
                        // Add empty string to have statement for break in the scope
                        scopeGraph->storeStmt(term, string());
                        // Removed loop break branch is run in separate analysis
                        breakInRemovedLoop = true;
                    }
                }
                
                // One successor block
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 1,
                                 "No one successor in break");
                
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Break Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                    //cout << "BREAK level "<< scopeGraph->getCurrLevel() << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(),
                                 "No successor reachable in break");
                
                // Get next scope for successor from @delayed or create it
                auto nextScope = getScopeForBlock(succBlock);
                // Set @nextScope as successor of the current scope
                scopeGraph->addScopeSucc(nextScope);
                
                // Check if the next block is loop input Phi function and 
                // current block is loop body input/outside input
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);
 
                // Next block is outside of the loop, so it at least one level up
                // Use loop scope level @currLoop.second here 
                // Use after wait flag for removed loop as it run in separate analysis
                ScopeInfo si(currLoop.level, block, scopeGraph->getCurrScope(), 
                             nextScope, loopInputBody, loopInputOut, loopStack, 
                             deadCode || breakInRemovedLoop, currLoop.stmt);
                blockSuccs.push_back({succBlock, si});
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "    block level "<< si.level <<", flags " << loopInputBody << ", " << loopInputOut << endl;
                }
            } else
            if ( isa<const ContinueStmt>(term) ) {
                if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                    if (!deadCode) term->dumpColor();
                }
                SCT_TOOL_ASSERT (!loopStack.empty(), 
                                 "Loop stack is empty in continue");

                // Parse and store continue statement, @termCondValue not used
                auto stmtStr = parseTerm(term, termCValue);
                
                // Generate @continue statement string
                auto& currLoop = loopStack.back();
                if (!deadCode) {
                    // Do not write @continue for removed loop
                    if (!currLoop.removed) {
                        scopeGraph->storeStmt(term, stmtStr.getValue());
                    } else {
                        // Add empty string to have statement for continue in the scope
                        scopeGraph->storeStmt(term, string());
                        // Removed loop continue branch is run in separate analysis
                        contInRemovedLoop = true;
                    }
                }

                // One successor block
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 1, 
                                 "No one successor in continue");
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Continue Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(),
                                 "No successor reachable in continue");
                
                // Get next scope for successor from @delayed or create it
                auto nextScope = getScopeForBlock(succBlock);
                // Set @nextScope as successor of the current scope
                scopeGraph->addScopeSucc(nextScope);

                // Check if the next block is loop input Phi function and 
                // current block is loop body input/outside input
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);
 
                // Next block is inside of the loop, so it at least one level up
                // Use after wait flag for removed loop as it run in separate analysis
                ScopeInfo si(scopeGraph->getCurrLevel(), block, 
                             scopeGraph->getCurrScope(), 
                             nextScope, loopInputBody, loopInputOut, loopStack,
                             deadCode || contInRemovedLoop);
                blockSuccs.push_back({succBlock, si});
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "    block level "<< si.level << ", flags " << loopInputBody << ", " << loopInputOut << endl;
                }
            } else    
            if (isa<const GotoStmt>(term)) {
                ScDiag::reportScDiag(term->getSourceRange().getBegin(), 
                                     ScDiag::CPP_GOTO_STMT);
            } else {
                string stmtClassName = term->getStmtClassName();
                ScDiag::reportScDiag(term->getSourceRange().getBegin(), 
                                     ScDiag::CPP_UNKNOWN_STMT) << stmtClassName;
            }
            
            // Clear called functions as they cannot be used after terminator
            // except ||/&&/? operators
            if ( !isa<BinaryOperator>(term) && !isa<ConditionalOperator>(term) ) {
                calledFuncs.clear();
            }

        } else {
            // Block has one successor
            if (cfgBlock->succ_size() == 1) {
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(), "No reachable successor");

                // Get next scope for successor from @delayed or create it
                auto nextScope = getScopeForBlock(succBlock);
                // Set @nextScope as successor of the current scope
                scopeGraph->addScopeSucc(nextScope);

                // Check if the next block is loop input Phi function and 
                // current block is loop body input/outside input
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);

                // Level not changed in general block, level up provided
                // for next block if it has multiple inputs 
                ScopeInfo si(scopeGraph->getCurrLevel(), block, 
                             scopeGraph->getCurrScope(), nextScope,
                             loopInputBody, loopInputOut, loopStack, deadCode);
                blockSuccs.push_back({succBlock, si});
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "    block level " << si.level << ", flags " << loopInputBody << ", " << loopInputOut << endl;
                }
            } else 
            if (cfgBlock->succ_size() > 1) {
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Block " << cfgBlock->getBlockID() << " has " << block.getCfgBlock()->succ_size() << " successors" << endl;
                }
                SCT_TOOL_ASSERT (false, "Too many successors");

            } else {
                // Do nothing if no successor, exit block has no successors
                SCT_TOOL_ASSERT (cfgBlock->getBlockID() == exitBlockId,
                                 "Block with no successor is not exit block");
                // Any function has one exit block, which is analyzed last,
                // so @state contains the final state
                if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                    cout << "----------------------------------------" << endl;
                    cout << "No more successors, exit from function "  << funcDecl->getNameAsString() << endl;
                    cout << "----------------------------------------" << endl;
                }
            }
        }

        // Store block successors, @block is unique in this call of @run()
        for (auto&& bs : blockSuccs) {
            bool found = false;
            auto i = delayed.rbegin();
            
            for (; i != delayed.rend(); ++i) {
                if (i->first == bs.first) {
                    found = true;
                    vector<ScopeInfo>& preds = i->second;
                    preds.push_back(bs.second);
                    
                    if (!i->first.isReachable() && bs.first.isReachable()) {
                        // Join reachability by OR, replace unreachable block 
                        // with reachable, not required if level is corrected
                        delayed.insert((++i).base(), {bs.first, preds});
                        delayed.erase(i.base());
                    }
                    
                    //cout << "Add pred in delayed for Block B" << AdjBlock::getCfgBlock(bs.first)->getBlockID() << endl;
                    break;
                }
            }
            // Insert new scope according with its level
            if (!found) {
                // The scope level, block after loop level is less than blocks
                // in the loop body, so loop body blocks analyzed first
                unsigned level = bs.second.level + ((bs.second.upLevel) ? 1 : 0);
                auto j = delayed.begin();
                for (; j != delayed.end(); ++j) {
                    if (level < j->second.begin()->level) {
                        break;
                    }
                }
                // Insert new scope before element pointed by @j
                delayed.insert(j, {bs.first, vector<ScopeInfo>(1, bs.second)});

                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Add to delayed new Block B" << AdjBlock::getCfgBlock(bs.first)->getBlockID() 
                         << " dead code/dead cond " << bs.second.deadCode << "/" << bs.second.deadCond << endl;
                    cout << "delayed size is " << delayed.size() << endl;
                }
            }
        }

        // Get next block from @delayed
        if (!delayed.empty()) {
            // Take block from @delayed
            auto i = delayed.rbegin();
            
            if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                cout << endl << "------- Choosing next block form delayed, size = " 
                     << delayed.size() << endl;
            }
            
            for (; i != delayed.rend(); ++i) {
                // Consider possible unreachable block
                unsigned predSize  = getPredsNumber(i->first);
                
                // Loop input from body/outside
                bool loopInputBody = i->second.begin()->loopInputBody;
                bool loopInputOut  = i->second.begin()->loopInputOut;
                
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    //cout << "nextCfgBlock B" << nextCfgBlock->getBlockID()
                    //     << ", preds/inputs " << predSize << "/" << i->second.size()
                    //     << ", flags " << loopInputBody << loopInputOut << endl;
                }
                    
                // Check number of ready inputs equals to predecessor number
                // At loop input from body only one body predecessor ready
                // At loop input from outside body predecessor not ready
                if (i->second.size() >= predSize || 
                    (loopInputBody && i->second.size() >= 1) ||
                    (loopInputOut && i->second.size() >= predSize-1))
                {
                    // Number of ready inputs can exceed predecessor number as
                    // duplicate predecessor is possible from dead code in THREADs
                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << endl << "Next BLOCK B" << i->first.getCfgBlockID() 
                             <<", scope " << hex << i->second.front().currScope.get() << dec
                            << ", flags "<< loopInputBody << ", " << loopInputOut 
                            << ", ready inputs " << i->second.size() << ", preds " << predSize << endl;
                    }
                    
                    // Prepare next block
                    prepareNextBlock(i->first, i->second);
                    break;
                }
            }
            
            // Check there is ready block
            if (i == delayed.rend()) {
                if (noreadyBlockAllowed) {
                    // Take last (most inner) block from @delayed
                    i = delayed.rbegin();
                    // Consider possible unreachable block
                    CFGBlock* nextCfgBlock = AdjBlock::getCfgBlock(i->first);
                    
                    // Required for THREAD wait-to-wait analysis
                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << endl << "Next (not ready) BLOCK B" << nextCfgBlock->getBlockID() 
                             << ", scope " << hex << i->second.front().currScope.get() << dec
                             << ", ready inputs " << i->second.size() << endl;
                    }    
                    
                    // Prepare next block
                    prepareNextBlock(i->first, i->second);
                    
                } else {
                    if (term) {
                        cout << term->getSourceRange().getBegin().printToString(sm) << endl;
                    } else 
                    if (currStmt) {
                        cout << currStmt->getSourceRange().getBegin().printToString(sm) << endl;
                    }
                    SCT_TOOL_ASSERT (false, "No ready block in @delayed");
                }
            }

            // Remove block from @delayed, use ++ as @base() points to next object 
            delayed.erase((++i).base());

        } else {
            // No more blocks to analysis 
            if (contextStack.empty()) {

                // Exit if no more context to analyzes
                if (storedContextStacks.empty()) {
                    // Cannot leave this function as there is break/continue
                    // branch to run separate analysis
                    SCT_TOOL_ASSERT (!breakInRemovedLoop && !contInRemovedLoop,
                                     "Exit from function with break/continue");
                    break;
                }
                
                // Get next stored context stack
                contextStack = storedContextStacks.back();
                storedContextStacks.pop_back();
            }
            
            // Restore callee function context, set skip function call element in block
            bool breakContext = restoreContext(true);
            skipOneElement = !breakContext;
        }
        
        // Store context in @break/@continue and run separate analysis
        // for @break/@continue, these flags not set after wait
        // Return to stored context after @break/@continue finished
        if (breakInRemovedLoop || contInRemovedLoop) {
            // Store current context at the start of next chosen block
            // @deadCode updated for next chosen block in prepareNextBlock() 
            lastContext = std::make_shared<ScFuncContext>(
                CfgCursor(funcDecl, block.getCfgBlock(), 0),
                returnValue, modval, recval, delayed, loopStack, 
                calledFuncs, /*funcLevel,*/ deadCode, true, scopeGraph, 
                codeWriter->serialize());
            
            string stmtName = ((breakInRemovedLoop) ? "break" : "continue");
            if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                cout << "--------------------------------------" << endl;
                cout << "Start analysis in "<< stmtName << " (removed loop)" << endl;
                cout << term->getSourceRange().getBegin().printToString(sm) << endl;
                cout << "--------------------------------------" << endl;
                cout << "block #" << block.getCfgBlockID() << ", deadCode " << deadCode << endl;
                //contextStack.printCursorStack();
            }

            // Store current context stack to analyze it after break/continue done
            contextStack.push_back( *(lastContext.get()) );
            ScProcContext currContextStack = contextStack;
            storedContextStacks.push_back(currContextStack); // TODO: add std::move
            lastContext = nullptr;

            // Clone context to have separate scope graphs connected in chain
            contextStack = contextStack.clone();
            // Add first scope graph in stored context scope graph,
            // used to print break/continue analysis statements
            auto firstGraph = contextStack.begin()->scopeGraph;
            firstGraph->setName(stmtName, false);
            // In @scopeGraph is last scope graph in the stored context stack
            // Loop statement not required for break/continue
            scopeGraph->addFCallScope(term, nullptr, firstGraph);

            // Set current scope graph for break/continue as last scope graph
            scopeGraph = contextStack.back().scopeGraph;
            contextStack.pop_back();
            // Clear delayed branches in all function context in stack
            for (auto& i : contextStack) {
                i.delayed.clear();
            }
            delayed.clear();
            
            // Set block to block after @break/@continue
            SCT_TOOL_ASSERT (blockSuccs.size() == 1, "No one successor found");
            prevBlock = block;
            block = blockSuccs.front().first;
            deadCode = false;
            
            // For @break remove current loop from stack
            if (breakInRemovedLoop) {
                SCT_TOOL_ASSERT (!loopStack.empty(), 
                                 "Loop stack is empty at break");
                loopStack.pop_back();
            }
            
            skipOneElement = false;
        }
    }
    
}

// Run for function declaration, the same as @setFunction() and @run()
void ScTraverseProc::run(clang::FunctionDecl* fdecl, bool emptySensitivity)
{
    setEmptySensitivity(emptySensitivity);
    setFunction(fdecl);
    run();
}

// Run for context stack, the same as @setContextStack() and @run()
void ScTraverseProc::run(const ScProcContext& cntxStack)
{
    setContextStack(std::move(cntxStack));
    run();
}

// Run for SVA property declaration, result string returned in success
llvm::Optional<std::string> ScTraverseProc::runSvaDecl(const FieldDecl* fdecl) 
{
    codeWriter->startStmt();
    const Stmt* stmt = parseSvaDecl(fdecl);
    auto stmtStr = codeWriter->getStmtString(stmt);
    
    if (stmtStr) {
        std::string s = (assertName ? (*assertName+" : ") : "") +
                        "assert property (\n    " + *stmtStr + " );\n";
        return s;
    }
    
    return llvm::None;
}

// Print function local variable declarations
void ScTraverseProc::printLocalDeclarations(std::ostream &os) 
{
    codeWriter->printLocalDeclaration(os, state.get());
}

// Print function local combinational variable declarations (for reset section)
void ScTraverseProc::printResetDeclarations(std::ostream &os)
{
    // Print local combinational variable declaration 
    codeWriter->printResetCombDecl(os);
    
    // Local declaration for member combinational variable modified in reset, 
    // required to avoid modification of the variable in two processes
    for (const SValue& val : codeWriter->getExtrCombVarUsedRst()) {
        SValue var = state->getVariableForValue(val);
        SCT_TOOL_ASSERT (var.isVariable(), "No variable for extrCombVarUsedRst");
        SValue pval = var.getVariable().getParent();
        
        // If variable has another parent it can be in MIF/record array, 
        // collect MIF/record array sizes to add in reset section declaration
        string indx;
        if (pval != modval) {
            SValue pvar = state->getVariableForValue(pval);
            SCT_TOOL_ASSERT (pvar.isVariable(),
                             "No parent variable for extrCombVarUsedRst");

            vector<size_t> arrSizes = getArraySizes(pvar.getType());
            for (size_t i : arrSizes) {
                indx += '[' + to_string(i) + ']';
            }
        }
        
        codeWriter->printDeclString(os, var, indx);
    }
}

// Print function statements
void ScTraverseProc::printFunctionBody(std::ostream &os) 
{
    // Scope graph for most top function (process function) 
    std::shared_ptr<ScScopeGraph> topGraph;
    if (contextStack.size() > 0) {
        topGraph = contextStack.at(0).scopeGraph;
    } else {
        topGraph = scopeGraph;
    }
    topGraph->putNotReplacedVars(codeWriter->getNotReplacedVars());
    topGraph->putVarAssignStmts(codeWriter->getVarAssignStmts());
    
    //cout << "--------------------------------------------" << endl;
    //cout << "Top function scopeGraph #" << hex << topGraph.get() << dec << endl;
    topGraph->printAllScopes(os); 
}

// Print temporal assertion in thread process, printed in @always_ff
void ScTraverseProc::printTemporalAsserts(std::ostream &os, bool forReset) 
{
    if (forReset) {
        for (auto str : sctRstAsserts) os << str; 
    } else {
        for (auto str : sctAsserts) os << str;
    }
}

// Get wait contexts
vector<ScProcContext>& ScTraverseProc::getWaitContexts() {
    return waitContexts;
}

// Get evaluated terminator condition values
// TODO: use std::move for #conds
void ScTraverseProc::setTermConds(const unordered_map<CallStmtStack, SValue>& conds) 
{
    termConds = conds;
    if (DebugOptions::isEnabled(DebugComponent::doConstResult)) {
        cout << "termConds size " << termConds.size() << endl;
        for (auto& i : termConds) {
            const Stmt* stmt = i.first.back();
            cout << "    " << getFileName(stmt->getSourceRange().getBegin().printToString(sm)) 
                 <<  " : " << i.second.asString() << endl;
        }
    }
}

// Current thread has reset signal
void ScTraverseProc::setHasReset(bool hasReset_)
{
    hasReset = hasReset_;
}

}