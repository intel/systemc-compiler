/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "sc_tool/cfg/ScTraverseConst.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/elab/ScElabDatabase.h"
#include "sc_tool/elab/ScVerilogModule.h"
#include "sc_tool/elab/ScObjectView.h"
#include "sc_tool/utils/CppTypeTraits.h"

#include "clang/AST/Decl.h"
#include "sc_tool/expr/ScParseExprValue.h"
#include <memory>

namespace sc {

using namespace std;
using namespace clang;
using namespace llvm;

// ---------------------------------------------------------------------------
// Auxiliary functions

// Extract counter variable value for given FOR initialization sub-statement
// \return variable value or NO_VALUE
SValue ScTraverseConst::extractCounterVar(const Stmt* stmt) 
{
    if (auto declStmt = dyn_cast<const DeclStmt>(stmt)) {
        SCT_TOOL_ASSERT (declStmt->isSingleDecl(), 
                         "Declaration group not supported");
        auto decl = dyn_cast<ValueDecl>(declStmt->getSingleDecl());
        SCT_TOOL_ASSERT (decl, "No ValueDecl in DeclStmt");
        
        SValue var(decl, modval); 
        return var;
    }
    return NO_VALUE;
}

// Evaluate terminator condition if it is compile time constant
void ScTraverseConst::evaluateTermCond(const Stmt* stmt, SValue& val) 
{
    // Initialization section FOR loop analyzed before the FOR terminator 
    // Get condition for terminator, LSH for binary &&/||
    auto cond = getTermCond(stmt);
    
    // Consider only integer value in condition as pointer is casted to bool
    if (cond) {
        evaluateConstInt(const_cast<Expr*>(cond), val, false);
        
        // Get condition variable to add Read property to @state
        // Required for single variable, for expression it added in the 
        // correspondent Binary/Unary statement
        SValue cval = evalSubExpr(const_cast<Expr*>(cond));
        state->readFromValue(cval);
    }
    
    // Set FOR counter variable to NO_VALUE if condition is unknown
    if (val.isUnknown()) {
        if (auto forstmt = dyn_cast<const ForStmt>(stmt)) {
            if (auto expr = forstmt->getInc()) {
                
                if (auto unrstmt = dyn_cast<UnaryOperator>(expr)) {
                    expr = unrstmt->getSubExpr();
                    
                } else {
                    if (auto cleanups = dyn_cast<ExprWithCleanups>(expr)) {
                        expr = cleanups->getSubExpr();
                    }
                    if (auto bindtmp = dyn_cast<CXXBindTemporaryExpr>(expr)) {
                        expr = bindtmp->getSubExpr();
                    }
                    if (auto opcall = dyn_cast<CXXOperatorCallExpr>(expr)) {
                        auto args = opcall->getArgs();
                        SCT_TOOL_ASSERT (opcall->getNumArgs(), 
                                         "Operator without arguments");
                        expr = const_cast<Expr*>(args[0]);
                    }
                    if (auto castexpr = dyn_cast<CastExpr>(expr)) {
                        expr = castexpr->getSubExpr();
                    }
                }   
                
                if (auto decl = dyn_cast<DeclRefExpr>(expr)) {
                    // Counter variable
                    SValue lval(decl->getDecl(), modval);
                    SValue rval = getValueFromState(lval);
                    state->putValue(lval, NO_VALUE);
                    
                    if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                        cout << "FOR loop replace counter " << lval.asString() 
                         << " = " << rval.asString() << " to NO_VALUE" << endl;
                    }
                }
                
            } else {
                // Empty increment, do nothing
            }
        }
    }
}

// Evaluate loop iteration number from conditional expression
llvm::Optional<unsigned> ScTraverseConst::evaluateIterNumber(const Stmt* stmt) 
{ 
    auto cond = getTermCond(stmt);
    if (cond) {
        if (auto binoper = dyn_cast<BinaryOperator>(cond) ) {
            BinaryOperatorKind opcode = binoper->getOpcode();
            
            if (opcode == BO_NE || opcode == BO_GT || opcode == BO_LT || 
                opcode == BO_GE || opcode == BO_LE) 
            {
                SValue rval;
                if (auto intLiter = dyn_cast<IntegerLiteral>(binoper->getRHS())) {
                    ScParseExpr::parseExpr(intLiter, rval);
                } else 
                if (auto intLiter = dyn_cast<IntegerLiteral>(binoper->getLHS())) {
                    ScParseExpr::parseExpr(intLiter, rval);
                    }
                if (rval.isInteger()) {
                    unsigned iterNumber = rval.getInteger().getZExtValue();
                    return iterNumber;
                }
            }
        }
    }
    return llvm::None;
}

// Check if this loop needs compare state with last iteration state
// \param iterCntr -- number of analyzed iteration
bool ScTraverseConst::isCompareState(const Stmt* stmt, unsigned maxIterNumber, 
                                     unsigned iterNumber, unsigned iterCntr) 
{
    if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
        cout << "CompareState iterNumber " << ((iterNumber) ? 
                to_string(iterNumber) : "unknown") << endl;
    }
    
    if (iterNumber && iterNumber < maxIterNumber-1) {
        // Loop with known and not too big iteration number 
        bool result = (iterCntr == iterNumber+1 || iterCntr >= maxIterNumber);
        if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
            cout << "CompareState fixed iteration loop " << iterCntr << "/" 
                 << iterNumber << ", need to compare " << result << endl;
        }
        return result;
                
    } else {
        // Loop with unknown or too big iteration number 
        bool result = (iterCntr == COMPARE_STATE_ITER1 || 
                       iterCntr == COMPARE_STATE_ITER2 || 
                       iterCntr >= maxIterNumber);
        if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
            cout << "CompareState unknown/big iteration loop, iterCntr " 
                 << iterCntr << " need to compare " << result << endl;
        }
        return result;
    }
    
    return false;
}

// Check then/else branches are empty
bool ScTraverseConst::checkIfBranchEmpty(const Stmt* branch) 
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
void ScTraverseConst::prepareNextBlock(AdjBlock& nextBlock, 
                                       vector<ConstScopeInfo>& scopeInfos) 
{
    // Taken block must have any predecessor
    SCT_TOOL_ASSERT (!scopeInfos.empty(), "No predecessor for next block");
    
    // Update predecessor scopes and return loop stack for the next block    
    loopStack = getLoopStack<ConstLoopStack>(scopeInfos);

    // Check for switch case block
    Stmt* lblStmt = nextBlock.getCfgBlock() ? 
                    nextBlock.getCfgBlock()->getLabel() : nullptr;
    bool switchCase = lblStmt && isa<SwitchCase>(lblStmt);
    
    unsigned currLoopLevel = (loopStack.size()) ? loopStack.back().level : 0;
    // If there is input from @continue, use current loop level+1, 
    // +1 as it is inside of the loop and level for FOR/WHILE is decreased below
    // TODO: fix me, remove +1
    auto levelPair = calcNextLevel(scopeInfos, currLoopLevel+1, emptyCaseBlocks, 
                                   switchCase);
    //~TODO
    unsigned nextLevel = levelPair.first;
    unsigned upLevel = levelPair.second;
    
    // Block with FOR/WHILE terminator has @level, and the loop body @level+1
    // Block with DO..WHILE terminator and and the loop body have @level+1
    // Update block level for FOR/WHILE and DO..WHILE loop
    CFGBlock* cfgBlock = nextBlock.getCfgBlock();
    const Stmt* term = (cfgBlock->getTerminator().getStmt()) ? 
                        cfgBlock->getTerminator().getStmt() : nullptr;
    // Decrease FOR/WHILE loop entry from the loop body
    if (term && (isa<const ForStmt>(term) || isa<const WhileStmt>(term))) {
        bool loopVisited = loopStack.isCurrLoop(term);
        if (loopVisited) {
            nextLevel -= 1;
        }
    }
    // Increase DO entry block level 
    if (auto doterm = getDoWhileTerm(nextBlock)) {
        bool loopVisited = loopStack.isCurrLoop(doterm);
        if (!loopVisited) {
            nextLevel += 1;
        }
    }

    // Erase local variables in Phi functions
    if (upLevel) {
        state->removeValuesByLevel(nextLevel);
    }
    
    // Set previous and new current block
    prevBlock = block;
    block = nextBlock;
    level = nextLevel;

    // Join dead code flags by AND form all inputs 
    deadCode = true;
    deadCond = true;
    for (auto&& si : scopeInfos) {
        deadCode = deadCode && si.deadCode;
        deadCond = deadCond && si.deadCond;
    }
    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
        cout << "  dead code/dead cond " << deadCode << "/" << deadCond 
             << " level " << level << endl;
    }
    
    if (!isCombProcess) {
        // Join visited loops by OR
        visitedLoops.clear();
        for (auto&& si : scopeInfos) {
            if (!si.deadCode) {
                visitedLoops.insert(si.visitedLoops.begin(), 
                                    si.visitedLoops.end());
            }
        }
        // Remove loop exit statements
        for (auto&& si : scopeInfos) {
            if (si.loopExit) {
                visitedLoops.erase(si.loopExit);
            }
        }
    }
    
    // Join states
    auto si = scopeInfos.begin();
    state = si->state;
    for (++si; si != scopeInfos.end(); ++si) {
        state->join(si->state.get());
    }
}

// ---------------------------------------------------------------------------
// Parsing functions overridden

// Register return value and prepare @lastContext
void ScTraverseConst::prepareCallContext(Expr* expr, 
                                         const SValue& funcModval, 
                                         const SValue& funcRecval, 
                                         const clang::FunctionDecl* callFuncDecl,
                                         const SValue& retVal) 
{
//    expr->dumpColor();
//    cout << "prepareCallContext " << hex << expr << dec 
//         << ", calledFuncs.size = " << calledFuncs.size() << " retVal " 
//         << retVal << endl;

    // Store return value for the call expression to replace where it used
    if (calledFuncs.count(expr) == 0) {
        calledFuncs.emplace(expr, retVal);
    } else {
        cout << hex << expr << dec << endl;
        SCT_TOOL_ASSERT (false, "Second meet of function call");
    }

    // Prepare context to store
    lastContext = std::make_shared<ConstFuncContext>(
                            CfgCursor(funcDecl, nullptr, 0), 
                            returnValue, modval, recval,
                            delayed, loopStack, calledFuncs, level, deadCode, 
                            stmtStoredValue);
    
    // Set module, dynamic module and return value for called function
    modval = funcModval;
    recval = funcRecval;
    returnValue = retVal;
    funcDecl = callFuncDecl;
}

// Parse and return integer value of wait(...) argument
unsigned ScTraverseConst::parseWaitArg(clang::CallExpr* expr)
{
    // Get arguments
    Expr** args = expr->getArgs();
    unsigned argNum = expr->getNumArgs();

    if (argNum > 0) {
        // TODO : check if argument is sc_event 
        if (argNum > 1) {
            ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::SC_UNSUPP_WAIT_KIND);
            return 1;
        } else 
        // Check argument type is integer, elsewhere it is wait()
        if ( !isAnyInteger(args[0]->getType()) ) {
            return 1;
        }
        
        SValue wval; chooseExprMethod(args[0], wval);
        SValue wwval = getValueFromState(wval);

        if (wwval.isInteger()) {
            const APSInt& waitNVal = wwval.getInteger();

            if (waitNVal.isNullValue() || waitNVal.isNegative()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SC_WAIT_N_NONPOSITIVE);
                return 1;
                
            } else {
                //cout << "wait(N) " << wval << " " << wwval << endl;
                return waitNVal.getZExtValue();
            }
        } else {
            //cout << wval << " " << wwval << endl;
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SC_WAIT_N_VARIABLE);
            return 1;
        }
    } else {
        return 1;
    }
}


// Function call expression
void ScTraverseConst::parseCall(CallExpr* expr, SValue& val) 
{
    ScParseExprValue::parseCall(expr, val);
    // Return value passed in @val
    SValue retVal = val;

    // Get arguments
    Expr** args = expr->getArgs();
    unsigned argNum = expr->getNumArgs();

    // Get function/method
    // Get function name and type
    FunctionDecl* funcDecl = expr->getDirectCallee();
    SCT_TOOL_ASSERT (funcDecl, "No function found for call expression");
    string fname = funcDecl->getNameAsString();
    auto nsname = getNamespaceAsStr(funcDecl);

    if (nsname && *nsname == "sc_core" && fname == "wait") {
        waitCall = parseWaitArg(expr);
        
    } else     
    if (fname == "sct_assert_level") {
        // Checking level assertion in regression tests
        SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");

        SValue lval = evalSubExpr(args[0]);
        SCT_TOOL_ASSERT (lval.isInteger(), 
                        "Only integer literal supported in sct_assert_level");
        unsigned expectedLevel = lval.getInteger().getZExtValue();
       
        if (level != expectedLevel) {
            cout << "--------------------------" << endl;
            cout << "Incorrect level " << level << " expected " << expectedLevel << endl;
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(),
                                 ScDiag::CPP_ASSERT_FAILED);
            SCT_TOOL_ASSERT (false, "Incorrect level stop");
        }
        val = NO_VALUE; // No value returned from assert

    } else
    if (fname == "sct_assert") {
        // Do nothing
        
    } else 
    if (fname == "sct_assert_const" || fname == "__assert" || 
        fname == "__assert_fail" || 
        (nsname && (*nsname == "sc_core" || *nsname == "sc_dt"))) {
        // Do nothing, implemented in ScParseExprValue
        
    } else 
    if (fname == "sct_assert_defined" || fname == "sct_assert_register" ||
        fname == "sct_assert_read" || fname == "sct_assert_latch") {
        // Do nothing, implemented in ScParseExprValue
        
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
        //cout << "ScTraverseConst::parseCall ignore function " << fname << endl;
        
    } else {
        // General function call
        if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for FUNCTION : " << fname << " (" 
                 << expr->getSourceRange().getBegin().printToString(sm) << ") |" << endl;
            cout << "-------------------------------------" << endl;
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
void ScTraverseConst::parseMemberCall(CXXMemberCallExpr* expr, SValue& val) 
{
    ScParseExprValue::parseMemberCall(expr, val);
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
        //cout << "ScTraverseConst::parseMemberCall ignore function " << fname << endl;
        
    } else
    if ( isAnyScIntegerRef(thisType, true) ) {
        // Do nothing yet
        
    } else 
    if ( isScChannel(thisType) ) {
        // Do nothing, all logic implemented in ScParseExprValue
        
    } else 
    if (isScCoreObject && fname == "wait" ) {
        // SC wait call
        waitCall = parseWaitArg(expr);
        
    } else
    if (isScCoreObject && fname == "name" ) {
        // Do nothing for specific @sc_core functions
        
    } else {
        // General method call
        if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for METHOD FUNCTION : " << fname << " (" 
                 << expr->getSourceRange().getBegin().printToString(sm) 
                 << ") |" << endl;
            cout << "-------------------------------------" << endl;
        }

        // Get value for @this
        SValue tval;
        chooseExprMethod(thisExpr, tval);
        
        // Get record from variable/dynamic object
        SValue ttval = getRecordFromState(tval, ArrayUnkwnMode::amFirstElement);
        
        // Allowed parent kinds
        if (!ttval.isArray() && !ttval.isRecord() && !ttval.isVariable()) {
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(),
                                 ScDiag::SYNTH_INCORRECT_RECORD);
            ttval = NO_VALUE;
        }

        // Call with cast this object to specific class with "::",
        // function call is not virtual in this case
        bool hasClassCast = false;
        if (auto memberExpr = dyn_cast<MemberExpr>(expr->getCallee())) {
            hasClassCast = memberExpr->hasQualifier();
        }
        
        // Get dynamic class for member record
        SValue dyntval = ttval; 
        // @modval for called function analysis
        SValue funcModval = ttval;

        // Get best virtual function and its dynamic class for @funcDecl
        if (funcDecl->isVirtualAsWritten() && !hasClassCast) {
//            if (!dyntval.isRecord()) {
//                //funcDecl->getSourceRange().getBegin().dump(sm);
//                thisExpr->dumpColor();
//                cout << "parseMemberCall tval " << tval << " " << isArr << unkwIndex << endl;
//                cout << "ttval " << ttval << " dyntval " << dyntval << endl;
//
//                //bool a = state->getBottomArrayForAny(tval, unkwIndex);
//                // 
//                ttval = state->getFirstArrayElementForAny(tval, false);
//            }
            
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

        // Check for array access at unknown index
        bool unkwIndex;
        bool isArr = state->isArray(tval, unkwIndex);
        // @tval can be field or array element
        isArr = isArr || state->getBottomArrayForAny(tval, unkwIndex);

        // Use array element with unknown index as module value to clear
        // array elements accessed in called function
        SValue fval = (isArr && unkwIndex) ? tval : funcModval;
        
        if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
            cout << "Function call this class value " << ttval
                 << ", dynamic class value " << dyntval
                << ", funcModval " << fval << endl;
        }
        
        // Generate function parameter assignments
        prepareCallParams(expr, fval, funcDecl);
        // Register return value and prepare @lastContext
        prepareCallContext(expr, fval, NO_VALUE, funcDecl, retVal);
        // Return value variable has call point level
        state->setValueLevel(retVal, level);
    }
}

// Choose and run DFS step in accordance with expression type.
// Remove sub-statements from generator
void ScTraverseConst::chooseExprMethod(Stmt* stmt, SValue& val)
{
    //cout << "--- chooseExprMethod " << hex << (unsigned long)stmt << dec << endl;
    // Constructor call for local record considered as function call
    bool anyFuncCall = isa<CallExpr>(stmt) || isa<CXXConstructExpr>(stmt);

    if (anyFuncCall && calledFuncs.count(stmt)) {
        val = calledFuncs.at(stmt);
        if (DebugOptions::isEnabled(DebugComponent::doConstStmt)) {
            cout << "Get RET value " << val.asString() << " for stmt #" << hex << stmt << dec  << endl;
        }

    } else {
        // Store sub-statement value to avoid re-calculation, 
        // required for conditions with side effects
        if (stmtStoredValue.count(stmt) == 0) {
            // First time meet @stmt in this function call, parse it
            ScParseExpr::chooseExprMethod(stmt, val);
            // Skip record constructor sub-statement without declaration, 
            // when @recval unknown, required to store @parseRecordCtor result
            bool skipRecCtor = isa<CXXConstructExpr>(stmt) && val.isUnknown();
            
            if (!skipRecCtor) {
                stmtStoredValue.emplace(stmt, val);
            }

        } else {
            // @stmt has been already analyzed
            val = stmtStoredValue.at(stmt);
        }
    }
}

// ------------------------------------------------------------------------
// Context functions

// Initialize analysis context at function entry
void ScTraverseConst::initContext() 
{
    SCT_TOOL_ASSERT (delayed.empty(), "@delayed is not empty");
    SCT_TOOL_ASSERT (loopStack.empty(), "@loopStack is not empty");
    SCT_TOOL_ASSERT (calledFuncs.empty(), "@calledFuncs is not empty");
    SCT_TOOL_ASSERT (stmtStoredValue.empty(), "@stmtStoredValue is not empty");
    SCT_TOOL_ASSERT (termConds.empty(), "@termCondResults is not empty");
    SCT_TOOL_ASSERT (funcDecl, "Function declaration and context stack not set");

    cfg = cfgFabric->get(funcDecl);
    block = AdjBlock(&cfg->getEntry(), true);
    prevBlock = block;
    level = 0;
    elemIndx = 0;
    exitBlockId = cfg->getExit().getBlockID();
    deadCode = false;
    deadCond = false;
    funcCall = false;
    
    // Setup first non-MIF module value 
    synmodval = state->getSynthModuleValue(modval, ScState::MIF_CROSS_NUM);

    // Check if current module if element of array of MIF
    zeroElmtMIF = false;
    nonZeroElmtMIF = false;
    if (isScModularInterface(modval.getType())) {
        // @modval is array element
        bool unkwIndex;
        SValue aval = state->getBottomArrayForAny(modval, unkwIndex, 
                                                  ScState::MIF_CROSS_NUM);

        if (aval.isArray()) {
            SValue mval;
            std::vector<int> indxs;
            state->getArrayIndices(aval, mval, indxs);
            
            mifElmtSuffix = "";
            for (auto i : indxs) {
                SCT_TOOL_ASSERT (i >= 0, "Unknown index for MIF array element");
                nonZeroElmtMIF = nonZeroElmtMIF || i != 0;
                mifElmtSuffix += "["+ to_string(i) +"]";
            }
            zeroElmtMIF = !nonZeroElmtMIF;
        }
    }
    
    state->clearValueLevels();
    
    if (DebugOptions::isEnabled(DebugComponent::doConstCfg)) {
        cout << "--------------------------------------" << endl;
        cout << "FUNCTION : " << funcDecl->getNameAsString() << endl;
        cfg->dump(LangOptions(), true);
        cout << "--------------------------------------" << endl;
    }
}

// Restore analysis context with given one
void ScTraverseConst::restoreContext() 
{
    /*cout << "Context stack :" << endl;
    for (auto& i : contextStack) {
        cout << "   scopeGraph #" << hex << i.scopeGraph.get() << dec << endl;
    }*/
        
    auto context = contextStack.back();
    contextStack.pop_back();
    loopStack = context.loopStack;
    modval = context.modval;
    recval = context.recval;
    returnValue = context.returnValue;
    funcDecl = context.callPoint.getFuncDecl();
    calledFuncs = context.calledFuncs;
    stmtStoredValue = context.stmtStoredValue;
    delayed = context.delayed;
    level = context.level;
        
    cfg = cfgFabric->get(funcDecl);
    SCT_TOOL_ASSERT (cfg, "No CFG at restore context");
    block = AdjBlock(const_cast<CFGBlock*>(context.callPoint.getBlock()), true);
    prevBlock = block;
    
    // Start analysis with restored element index
    elemIndx = context.callPoint.getElementID();
    exitBlockId = cfg->getExit().getBlockID();

    // Erase local variables of called function 
    state->removeValuesByLevel(level);
    
    // @deadCode not changed
    SCT_TOOL_ASSERT (!deadCode, "@deadCode in restore context");
    
    if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
        cout << "---------- restoreContext ------------" << endl;
        cout << "Level is " << level << ", modval " << modval << endl;
        //cfg->dump(LangOptions(), true);
        cout << "--------------------------------------" << endl;
        //state->print();
        //cout << "--------------------------------------" << endl;

        /*cout << " calledFuncs:" << endl;
        for (auto& i : calledFuncs) {
            cout << "   " << i.second.asString() << endl;
        }*/
    }
}

// Creates constant value in global state if necessary
SValue ScTraverseConst::parseGlobalConstant(const SValue &sVar)
{
    SValue res;

    if (res = state->getValue(sVar), res) {
        return res;
    }
    
    if (sVar.getVariable().getParent()) {
        // Member/local variable
        SValue constVal = ScParseExpr::parseGlobalConstant(sVar);
        return constVal;
    }

    if (elabDB == nullptr || globalState == nullptr ) {
        return ScParseExpr::parseGlobalConstant(sVar);
    }

    // Create new global variable
    SValue constVal = NO_VALUE;
    llvm::Optional<sc_elab::ObjectView> newElabObj;

    if (!globalState->getElabObject(sVar)) 
    {
        sc_elab::ModuleMIFView currentModule = *state->getElabObject(dynmodval);
        auto* varDecl = dyn_cast<clang::VarDecl>(sVar.getVariable().getDecl());
        // @exprEval used in createStaticVariable() to parse initializer
        ScParseExprValue exprEval{astCtx, state, modval};
        
        newElabObj = elabDB->createStaticVariable(currentModule, varDecl, exprEval);
        SCT_TOOL_ASSERT (newElabObj, "No static variable");

        if (elabDB->hasVerilogModule(currentModule)) {
            auto* verilogMod = elabDB->getVerilogModule(currentModule);
            verilogMod->addConstDataVariable(*newElabObj, 
                                             *(newElabObj->getFieldName()));
        } else {
            // Global/namespace constant or variable, may be need to register
        }

        // Parse and put initializer into global state
        auto valDecl = const_cast<clang::ValueDecl*>(sVar.getVariable().getDecl());
        ScParseExprValue globExprEval{astCtx, globalState, modval};
        globExprEval.parseValueDecl(valDecl, NO_VALUE, nullptr, false);

        // Put the same initializer into current state, to use it in the process
        // For next analyzed processes it is taken from @globalState
        exprEval.parseValueDecl(valDecl, NO_VALUE, nullptr, false);
        constVal = state->getValue(sVar);

        globalState->putElabObject(sVar, *newElabObj);
        
    } else {
        newElabObj = *globalState->getElabObject(sVar);
        constVal = globalState->getValue(sVar);
    }

    if (newElabObj) {
        state->putValue(sVar, constVal);
        // Put constant name into @state->extrValNames inside
        if (!state->getElabObject(sVar)) {
            state->putElabObject(sVar,*newElabObj);
        }
    }

    return constVal;
}

// Register variables accessed in and after reset section, 
// check read-not-defined is empty in reset
void ScTraverseConst::registerAccessVar(bool isResetSection, const Stmt* stmt) 
{
    if (isResetSection) {
        // Extract variables defined in reset section to create
        // declaration in module scope if it is constant
        SCT_TOOL_ASSERT (resetDefinedConsts.empty(), 
                         "resetDefinedValues is not empty");
        for (const auto& val : state->getDefArrayValues()) {
            if (val.isVariable() && ScState::isConstVarOrLocRec(val)) {
                resetDefinedConsts.insert(val);
            }
        }
        // Register variables accessed in CTHREAD reset 
        if (!isCombProcess) {
            //cout << "---- getDefArrayValues " << endl;
            for (const auto& val : state->getDefArrayValues()) {
                if (val.isVariable() || val.isTmpVariable()) {
                    inResetAccessVars.insert(val);
                    //cout << "   " << val << endl;
                }
            }
            //cout << "---- getReadValues " << endl;
            for (const auto& val : state->getReadValues()) {
                if (val.isVariable() || val.isTmpVariable()) {
                    inResetAccessVars.insert(val);
                    //cout << "   " << val << endl;
                }
            }
        }

        // Read-not-defined not valid at reset section,
        // get ReadNotDefined excluding read in SVA
        for (const auto& val : state->getReadNotDefinedValues(false)) {
            if (!val.isVariable()) continue;
            // Skip record field as copy constructor leads to 
            // read all the fields even if not really used
            if (ScState::isRecField(val)) continue;

            QualType type = val.getType();

            if (isScChannel(type, true) || isScChannelArray(type, true) ||
                ScState::isConstVarOrLocRec(val)) continue;

            ScDiag::reportScDiag(stmt->getBeginLoc(),
                    ScDiag::CPP_READ_NOTDEF_VAR_RESET) << val.asString(false);
        }

    } else {
        // Register variables accessed after CTHREAD reset
        if (!isCombProcess) {
            for (const auto& val : state->getDefArrayValues()) {
                if (val.isVariable() || val.isTmpVariable()) {
                    afterResetAccessVars.insert(val);
                }
            }
            for (const auto& val : state->getReadValues()) {
                if (val.isVariable() || val.isTmpVariable()) {
                    afterResetAccessVars.insert(val);
                }
            }
        }
    }
    
    // Register SVA used variables to create register for them
    for (const auto& val : state->getSvaReadValues()) {
        if (val.isVariable() || val.isTmpVariable()) {
            inSvaAccessVars.insert(val);
        }
    }
}

// ------------------------------------------------------------------------
// Main functions

// Preset function to run analysis, used for entry function analysis
void ScTraverseConst::setFunction(const FunctionDecl* fdecl)
{
    funcDecl = fdecl;
}

// Run analysis at function entry, runs once per process
void ScTraverseConst::run()
{
    //cout << endl << "====================== ScTraverseConst::run() ====================" << endl;
    
    // Initialize analysis context
    initContext();

    // Skip function call
    bool skipOneElement = false;
    // Is reset section analyzed 
    bool isResetSection = !isCombProcess && hasReset;
    
    while (true)
    {
        Stmt* currStmt = nullptr;
        // Unreachable block can be not dead code after &&/|| condition        
        // Do not generate statement for dead code block
        if (!deadCode && !deadCond) {
            //cout << "elemId = " << elemIndx << ", block size = " << block.getCfgBlock()->size() << endl;
            SCT_TOOL_ASSERT (elemIndx <= block.getCfgBlock()->size(),
                            "Incorrect elmId");

            // Update block level and loop stack for do...while loop enter
            if (auto doterm = getDoWhileTerm(block)) {
                // Determine loop already visited with checking if 
                // current (last) loop has the same terminator
                bool loopVisited = loopStack.isCurrLoop(doterm);
                if ( !loopVisited ) {
                    unsigned iterNumber = 0;
                    if (auto in = evaluateIterNumber(doterm)) {
                        iterNumber = in.getValue();
                    }
                    // Add current loop into loop stack, set 1st iteration
                    // Use level-1 as current block is inside the loop (level+1)
                    loopStack.pushLoop({doterm, level-1, 1U, state->clone(), 
                                        iterNumber});
//                    cout << "Push loop stack(1) " << hex << doterm << dec 
//                          << ", level " << level << ", LS size " << loopStack.size() << endl;
                }
            }
            
            // CFG block body analysis, preformed for not dead state only
            for (size_t i = elemIndx; i < block.getCfgBlock()->size(); ++i)
            {
                const CFGElement& elm = block.getCfgBlock()->operator [](i);
                if (elm.getKind() == CFGElement::Kind::Statement) {
                    // Get statement 
                    CFGStmt* s = elm.getAs<CFGStmt>().getPointer();
                    currStmt = const_cast<Stmt*>(s->getStmt());
                    
                    if (DebugOptions::isEnabled(DebugComponent::doConstStmt)) {
                        cout << endl;
                        currStmt->dumpColor();
                        cout << "level " << level << endl;
                        //state->print();
                    }
                    
                    // If started with restored context, move to the next element, 
                    // element stored in context was already analyzed
                    if (skipOneElement) {
                        if (DebugOptions::isEnabled(DebugComponent::doConstStmt)) {
                            cout << "SKIP this statement after restore context" << endl;
                        }
                        skipOneElement = false;
                        continue;
                    }
                    
                    // Parse statement and fill state
                    SValue val;
                    chooseExprMethod(currStmt, val);
                    
                    //state->print();
                    
                    // Wait call, store state and continue analysis
                    if (waitCall > 0) {
                        auto cursorStack = contextStack.getCursorStack();
                        
                        // Add current wait()
                        cursorStack.push_back(CfgCursor(funcDecl, 
                                              block.getCfgBlock(), i));
                        
                        // Add current function as function with wait()
                        hasWaitFuncs.insert(funcDecl);

                        // Register variables accessed in and after reset section
                        registerAccessVar(isResetSection, currStmt);
                        
                        if (!isCombProcess) {
                            // Register all loops as contain @wait(), do not use
                            // @loopStack here as it is cleaned at function call
                            for (auto& i : visitedLoops) {
                                waitInLoops.insert(i);
                            }
                            // Clear visited loop in wait()
                            visitedLoops.clear();
                        }
                        
                        // Get or create new wait state
                        SCT_TOOL_ASSERT (cthreadStates, 
                                         "No cthreadStates specified");
                        auto waitId = cthreadStates->getOrInsertStateID(
                                            cursorStack, waitCall);
                        
                        // Insert or join current state for this @wait()
                        auto i = waitStates.emplace(waitId, *(state.get()));
                        if (!i.second) {
                            i.first->second.join(state.get());
                        }
                        
                        waitCall = 0;
                        isResetSection = false;
                        
                        // Clean ReadDefined after wait()
                        state->clearReadAndDefinedVals();
                    }
                    
                    // Run analysis of called function in this traverse process
                    if (lastContext) {
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
                        
                        if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
                            cout << "--------------------------------------" << endl;
                            cfg->dump(LangOptions(), true);
                            cout << "--------------------------------------" << endl;
                        }
                        
                        // Store and null function context
                        contextStack.push_back( *(lastContext.get()) );
                        
                        // Start called function with empty @delayed and loop stack
                        delayed.clear();
                        loopStack.clear();
                        calledFuncs.clear();
                        stmtStoredValue.clear();
                        // Increase level for called function
                        level += 1;
                        
                        lastContext = nullptr;
                        funcCall = true;
                        break;
                    }
                } else {
                    SCT_TOOL_ASSERT (false, "Incorrect element kind");
                }
            }
        }

        // Start any block expect return to callee with first element
        elemIndx = 0;
        
        // Block successors
        vector<pair<AdjBlock, ConstScopeInfo> >  blockSuccs;
        CFGBlock* cfgBlock = block.getCfgBlock();
        const Stmt* term = (cfgBlock->getTerminator().getStmt()) ? 
                            cfgBlock->getTerminator().getStmt() : nullptr;
        
        if (funcCall) {
            // Suspend analysis of this function and go to called function
            funcCall = false;
            continue;
            
        } else 
        if (term) {
            // Erase value at loop entry level, therefore "-1" for DO..WHILE
            bool doWhile = isa<const DoStmt>(term);
            state->removeValuesByLevel((doWhile) ? level-1 : level);
                
            // Try to calculate condition as compile time
            SValue termCondValue;
            if (!deadCode) {
                // Get FOR-loop counter variable and register it in state
                if (auto forstmt = dyn_cast<const ForStmt>(term)) {
                    if (auto init = forstmt->getInit()) {
                        SValue cval = extractCounterVar(init);
                        state->regForLoopCounter(cval);
                    }
                }

                // Do not calculate condition for dead code as state is dead
                evaluateTermCond(term, termCondValue);
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    cout << "termCondValue " << termCondValue.asString() << endl;
                }
            }
            
            // Store terminator condition to use in ScTraverseProc, 
            // different results joined to NO_VALUE
            if (!deadCode) {
                auto callStack = contextStack.getStmtStack();
                callStack.push_back(term);
                // FOR/WHILE loop first iteration, used to get separate condition value
                bool loopFirstIter = (isa<const ForStmt>(term) || 
                        isa<const WhileStmt>(term)) && !loopStack.isCurrLoop(term);
                // Use call stack with double #term to distinguish first iteration
                if (loopFirstIter) {
                    callStack.push_back(term);
                }
                auto i = termConds.find(callStack);

                if (i == termConds.end()) {
                    termConds.emplace(callStack, termCondValue);
                } else {
                    if (i->second != termCondValue) {
                        i->second = NO_VALUE;
                    }
                }
            }
            
            if (const IfStmt* ifstmt = dyn_cast<const IfStmt>(term)) {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    if (!deadCode) term->dumpColor();
                }
                
                bool deadThen = termCondValue.isInteger() && 
                                termCondValue.getInteger().isNullValue();
                bool deadElse = termCondValue.isInteger() && 
                                !termCondValue.getInteger().isNullValue();
                
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2, 
                                 "No two successors in IF");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                // Then block always exists even if it is empty
                AdjBlock thenBlock(*iter);
                // If Else branch is empty, Else block is next IF block
                AdjBlock elseBlock(*(++iter));
                SCT_TOOL_ASSERT (thenBlock != elseBlock, 
                                 "The same then and else blocks in IF");

                // If Else branch is empty, Else block is next IF block
                bool elseIsEmpty = checkIfBranchEmpty(ifstmt->getElse()); 

                // Check if the next block is loop input (has loop terminator) 
                // and current block is loop body input/outside input
                bool thenLoopInputBody = checkLoopInputFromBody(block, thenBlock);
                bool thenLoopInputOut  = checkLoopInputFromOut(block, thenBlock);
                bool elseLoopInputBody = checkLoopInputFromBody(block, elseBlock);
                bool elseLoopInputOut  = checkLoopInputFromOut(block, elseBlock);
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "   thenBlock flags " << thenLoopInputBody << ", " << thenLoopInputOut << endl;
                    cout << "   elseBlock flags " << elseLoopInputBody << ", " << elseLoopInputOut << endl;
                    cout << "IF dead " << deadCode << " then/else " <<(deadCode || deadThen) << (deadCode || deadElse) << endl;
                }
                
                // Normal non-empty branch block has +1 level
                ConstScopeInfo thenES((deadThen) ? make_shared<ScState>(true) : 
                                      state, level+1, block, 
                                      thenLoopInputBody, thenLoopInputOut, 
                                      loopStack, deadCode || deadThen,
                                      visitedLoops);
                // If Else branch is empty, next block has level up even if 
                // there is no @Phi function, so set level up flag
                ConstScopeInfo elseES((deadElse) ? make_shared<ScState>(true) : 
                                      (deadThen) ? state : shared_ptr<ScState>(
                                                   state->clone()), level+1, 
                                      block, elseLoopInputBody, elseLoopInputOut, 
                                      loopStack, deadCode || deadElse, 
                                      visitedLoops, nullptr, elseIsEmpty);
                // Store ready to analysis block last, to get it first
                blockSuccs.push_back({elseBlock, elseES});
                blockSuccs.push_back({thenBlock, thenES});

            } else
            if (const SwitchStmt* swstmt = dyn_cast<const SwitchStmt>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    if (!deadCode) term->dumpColor();
                }

                // Add current switch into loop stack, no state stored
                loopStack.pushLoop({term, level, 1U, nullptr, 0});
                
                // Switch cases loop, skip first case as it belongs to @default
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
                
//                cout << "cases : " << endl;
//                for (auto entry : cases) {
//                    cout << "   " << hex << entry.first << dec << " block #"
//                         << entry.second.getCfgBlockID() << endl;
//                    //entry.first->dumpColor();
//                }

                bool allEmptyCases = true; // All cases including default are empty
                bool constCase = false; // One case chosen by constant condition
                bool clone = false; // Clone @state after first case
                
                // Cases loop
                for (auto entry : cases) {
                    const SwitchCase* swcase = entry.first;
                    AdjBlock caseBlock = entry.second;
                    SCT_TOOL_ASSERT (caseBlock.getCfgBlock(), "No switch case block");
                    
                    bool emptyCase = isCaseEmpty(caseBlock);
                    if (emptyCase) emptyCaseBlocks.insert(caseBlock);
                    allEmptyCases = allEmptyCases && emptyCase;
                    
                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                        cout << "CaseBlock B" << caseBlock.getCfgBlockID() 
                             << ((caseBlock.isReachable()) ? "" : " unreachable") << endl;
                        if (!caseBlock.isReachable()) {
                            cout << "Unreachable block at" << term->getSourceRange().
                                    getBegin().printToString(sm) << endl;
                        }
                    }
                    
                    SValue caseValue;
                    if (auto* cstmt = dyn_cast<const CaseStmt>(swcase)) {
                        // Get case expression and store it in generator
                        Expr* expr = const_cast<Expr*>(cstmt->getLHS());
                        // Evaluate case value
                        evaluateConstInt(expr, caseValue, false);
                        
                    } else {
                        SCT_TOOL_ASSERT (false, "Unexpected statement type in switch");
                    }
                    
                    bool loopInputBody = checkLoopInputFromBody(block, caseBlock);
                    bool loopInputOut  = checkLoopInputFromOut(block, caseBlock);

                    bool deadCase = false;
                    if (termCondValue.isInteger() && caseValue.isInteger()) {
                        deadCase = !APSInt::isSameValue(termCondValue.getInteger(), 
                                                        caseValue.getInteger());
                        constCase = constCase || !deadCase;
                    }
                    
                    // Case/default have +1 level, loop stack with this @switch
                    ConstScopeInfo caseSI((deadCase) ? make_shared<ScState>(true) : 
                            (clone) ? shared_ptr<ScState>(state->clone()) : state, 
                            level+1, block, loopInputBody, loopInputOut, 
                            loopStack, deadCode || deadCase, visitedLoops);
                    blockSuccs.push_back({caseBlock, caseSI});
                    clone = clone || !deadCase;
                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                        cout << "    block level " << caseSI.level << ", flags " << loopInputBody << ", " << loopInputOut << endl;
                    }
                }

                // Last successor block is default or after switch block
                AdjBlock succBlock(*cfgBlock->succ_rbegin());
                
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "Switch Default/Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                    if (!succBlock.isReachable()) {
                        cout << "Unreachable block at" << term->getSourceRange().getBegin().printToString(sm) << endl;
                    }
                }
                
                // Write switch code only if there are several branches
                bool writeSwitch = (cfgBlock->succ_size() > 1);
                
                unsigned nextLevel; 
                const Stmt* loopExit;

                if (hasDefault) {
                    // Default scope level +1 if there are case scopes
                    nextLevel = level + ((writeSwitch && !emptyDefault) ? 1 : 0);
                    allEmptyCases = allEmptyCases && emptyDefault;
                    loopExit = emptyDefault ? term : nullptr;
                    if (emptyDefault) emptyCaseBlocks.insert(succBlock);
                    
                } else {
                    // No default scope, add next block successor
                    nextLevel = level;
                    loopExit = term;
                }
                
                // Report all empty cases, not supported in @calcNextLevel()
                if (allEmptyCases) {
                    ScDiag::reportScDiag(swstmt->getBeginLoc(),
                                         ScDiag::SYNTH_SWITCH_ALL_EMPTY_CASE);
                    // To avoid further errors
                    SCT_INTERNAL_FATAL_NOLOC ("Empty switch without break not supported");
                }
                
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);
                
                // Set loop exit to avoid level up in taking @switch next block
                bool deadDefault = hasDefault && 
                                   (constCase || !succBlock.isReachable());
                ConstScopeInfo switchSI((deadDefault) ? make_shared<ScState>(true) : 
                        (clone) ? shared_ptr<ScState>(state->clone()) : state, 
                        nextLevel, block, loopInputBody, loopInputOut, loopStack, 
                        deadCode || deadDefault, visitedLoops, loopExit);
                blockSuccs.push_back({succBlock, switchSI});
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "    block level " << switchSI.level << ", flags " 
                         << loopInputBody << ", " << loopInputOut << endl;
                }

            } else 
            if (const BinaryOperator* binstmt = dyn_cast<const BinaryOperator>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    if (!deadCode) term->dumpColor();
                }

                BinaryOperatorKind opcode = binstmt->getOpcode();
                SCT_TOOL_ASSERT (opcode == BO_LOr || opcode == BO_LAnd, 
                                 "Incorrect terminator statement");

                // Then branch leads to right part of &&\||, else branch leads 
                // to whole expression 
                bool deadThen = opcode == BO_LAnd && termCondValue.isInteger() &&  
                                termCondValue.getInteger().isNullValue();
                bool deadElse = opcode == BO_LOr && termCondValue.isInteger() && 
                                !termCondValue.getInteger().isNullValue();
                
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in ||/&&");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock thenBlock(*iter);
                AdjBlock elseBlock(*(++iter));
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
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
                
                // Only path to final IF/Loop/? statement is taken, 
                // if this path is dead it is detected at the final statement
                // @deadThen/Else not consider @deadCond required for complex conditions
                if (opcode == BO_LOr) {
                    ConstScopeInfo elseSI(state, level, block,
                                          elseLoopInputBody, elseLoopInputOut, 
                                          loopStack, deadCode, visitedLoops,
                                          nullptr, false, deadElse);
                    blockSuccs.push_back({elseBlock, elseSI});
                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                        cout << "&&/|| elseBlock level " << elseSI.level << ", flags " << elseLoopInputBody << ", " << elseLoopInputOut << endl;
                    }
                }
                if (opcode == BO_LAnd) {
                    ConstScopeInfo thenSI(state, level, block,
                                          thenLoopInputBody, thenLoopInputOut, 
                                          loopStack, deadCode, visitedLoops,
                                          nullptr, false, deadThen);
                    blockSuccs.push_back({thenBlock, thenSI});
                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                        cout << "&&/|| thenBlock level " << thenSI.level << ", flags " << thenLoopInputBody << ", " << thenLoopInputOut << endl;
                    }
                }
                
            } else
            if ( isa<const ConditionalOperator>(term) )
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    if (!deadCode) term->dumpColor();
                }

                bool deadThen = termCondValue.isInteger() && 
                                termCondValue.getInteger().isNullValue();
                bool deadElse = termCondValue.isInteger() && 
                                !termCondValue.getInteger().isNullValue();
                
                // Two argument blocks    
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in cond operator");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock argBlock1(*iter);
                AdjBlock argBlock2(*(++iter));
                // Block in conditional operator can be unreachable
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    //cout << "? ArgBlock1 B" << argBlock1.getCfgBlockID() << ((argBlock1.isReachable()) ? "" : " is unreachable") << endl;
                    //cout << "? ArgBlock2 B" << argBlock2.getCfgBlockID() << ((argBlock2.isReachable()) ? "" : " is unreachable") << endl;
                    //cout << "? dead " << deadCode << (deadCode || deadThen) << (deadCode || deadElse) << endl;
                }
                
                // Arguments have +1 level to return the same level for ? body 
                // No loop terminator possible in argument blocks
                ConstScopeInfo argSI1((deadThen) ? make_shared<ScState>(true) : 
                                       state, level+1, block, false, false, 
                                       loopStack, deadCode || deadThen,
                                       visitedLoops);
                ConstScopeInfo argSI2((deadElse) ? make_shared<ScState>(true) : 
                                      (deadThen) ? state : shared_ptr<ScState>(
                                                   state->clone()), 
                                      level+1, block, false, false, loopStack, 
                                      deadCode || deadElse, visitedLoops);
                blockSuccs.push_back({argBlock1, argSI1});
                blockSuccs.push_back({argBlock2, argSI2});
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << " block1 level " << argSI1.level << endl;
                    cout << " block2 level " << argSI2.level << endl;
                }

            } else                 
            if (isa<const ForStmt>(term) || isa<const WhileStmt>(term) ||
                isa<const DoStmt>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    if (!deadCode) term->dumpColor();
                }
                
                // Determine loop already visited with checking if 
                // current (last) loop has the same terminator
                bool loopVisited = doWhile || loopStack.isCurrLoop(term);
                
                if (!isCombProcess && !deadCode) {
                    if (waitInLoops.count(term) && visitedLoops.count(term)) {
                        ScDiag::reportScDiag(term->getBeginLoc(), 
                                             ScDiag::SYNTH_WAIT_LOOP_FALLTHROUGH);
                        // Assert required to prevent analysis hangs up
                        SCT_INTERNAL_FATAL(term->getBeginLoc(), 
                                           "Loop fallthrough path w/o wait()");
                    }
                    visitedLoops.insert(term);
                }
                
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2, 
                                 "No two successors in loop");
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock bodyBlock(*iter);
                AdjBlock exitBlock(*(++iter));

                bool exitLoopInputBody = checkLoopInputFromBody(block, exitBlock);
                bool exitLoopInputOut  = checkLoopInputFromOut(block, exitBlock);
                bool enterLoopInputBody = (bodyBlock.getCfgBlock()) ? 
                            checkLoopInputFromBody(block, bodyBlock) : false;
                bool enterLoopInputOut  = (bodyBlock.getCfgBlock()) ? 
                            checkLoopInputFromOut(block, bodyBlock) : false;

                // Next block level
                // Block with FOR/WHILE has @level, and the loop body @level+1
                // Block with DO..WHILE and and the loop body have @level
                unsigned nextLevel = (doWhile) ? level : level+1;
                
                // Stop loop analysis and go to exit block if iteration exceeded
                bool maxIterFlag = false;
                // Stop loop analysis if state is not changed at last iteration
                bool stableState = false;
                
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    cout << "Loop terminator, level " << level << endl;
                }
                
                if (loopVisited) {
                    // Maximal iteration number to analyze
                    const unsigned loopMaxIter = 
                                (loopStack.size() <= DEEP_LOOP_DEPTH) ? 
                                 LOOP_MAX_ITER : DEEP_LOOP_MAX_ITER;
                    
                    // Check if state changed at some iterations
                    bool compareState = isCompareState(term, loopMaxIter,
                                            loopStack.back().iterNumber,
                                            loopStack.back().counter+1);
                    if (compareState) {
                        //state->print();
                        //loopStack.back().state->print();
                        stableState = state->compare(loopStack.back().state.get());
                        if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
                            cout << "Loop stable state " << stableState << endl;
                        }
                    }
                    
                    // Check and increment iteration counter
                    if (!stableState) {
                        if (loopStack.back().counter < loopMaxIter) {
                            // Normal iterations
                            if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
                                cout << "Loop next iteration " << hex << loopStack.back().stmt << dec 
                                     << ", counter " << loopStack.back().counter << endl;
                            }
                            loopStack.back().counter += 1;
                            // Store state for next comparison
                            if (compareState) {
                                loopStack.back().state = 
                                        shared_ptr<ScState>(state->clone());
                            }

                        } else 
                        if (loopStack.back().counter >= loopMaxIter && 
                            loopStack.back().counter < UNROLL_ERROR_ITER) 
                        {
                            // Several last iterations with replace to NO_VALUE,
                            // required to spread NO_VALUE through all conditions
                            if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
                                cout << "Loop last iteration(s) " << hex << loopStack.back().stmt << dec 
                                     << ", counter " << loopStack.back().counter << endl;
                                cout << "Set NO_VALUE for unstable state delta" << endl;
                            }
                            loopStack.back().counter += 1;
                            // Set NO_VALUE for all different tuples,
                            // at next iteration state should be stable
                            state->compareAndSetNovalue(
                                        loopStack.back().state.get());
                            // Store state for next comparison
                            SCT_TOOL_ASSERT(compareState, "No compare state flag");
                            loopStack.back().state = 
                                        shared_ptr<ScState>(state->clone());
                            
                        } else {
                            SCT_TOOL_ASSERT(compareState, "No compare state flag");
                            maxIterFlag = true;
                            ScDiag::reportScDiag(term->getSourceRange().getBegin(), 
                                                 ScDiag::SC_ERROR_CPROP_UNROLL_MAX);
                            state->print();
                        }
                    } else {
                        // If state is stable one more iteration required to ensure 
                        // it is really stable, but not eventual coincidence
                        if (loopStack.back().counter < LOOP_LAST_ITER) {
                            loopStack.back().counter = LOOP_LAST_ITER;
                            // Clear to have iteration through loop body
                            stableState = false;
                        }
                    }
                    
                } else {
                    if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
                        cout << "Push loop stack(2) " << hex << term << dec << ", level " << level << endl;
                    }
                    // Try to get iteration number
                    unsigned iterNumber = 0;
                    if (auto in = evaluateIterNumber(term)) {
                        iterNumber = in.getValue();
                    }
                    // Store state to compare with last iteration state
                    loopStack.pushLoop({term, level, 1U, state->clone(), 
                                        iterNumber});
                }
                
                // Add only one successor for visited loop with false condition,
                // loop body must be visited to get all break predecessors for
                // loop exit, terminator condition not evaluated for dead code
                bool oneSucc = maxIterFlag || stableState ||
                               (termCondValue.isInteger() && 
                               (!termCondValue.getInteger().isNullValue() ||
                                loopVisited)) || (deadCode && loopVisited);
                bool deadBody = termCondValue.isInteger() &&
                                termCondValue.getInteger().isNullValue();

                //cout << "Loop oneSucc " << oneSucc << ", loopVisited " << loopVisited 
                //     << ", deadBody " << deadBody << ", counter " << loopStack.back().counter << endl;
                
                // Use @term as loop exit
                ConstScopeInfo exitSI(state, nextLevel-1, block,
                                      exitLoopInputBody, exitLoopInputOut, 
                                      loopStack, deadCode, visitedLoops, term);
                ConstScopeInfo enterSI((oneSucc) ? state : shared_ptr<ScState>(
                                       state->clone()), nextLevel, block,
                                       enterLoopInputBody, enterLoopInputOut, 
                                       loopStack, deadCode || deadBody,
                                       visitedLoops);
                
                if (oneSucc) {
                    if (maxIterFlag || stableState || deadCode ||
                        termCondValue.getInteger().isNullValue()) {
                        // Go to loop exit, check if exit block exists
                        if (exitBlock.getCfgBlock()) {
                            blockSuccs.push_back({exitBlock, exitSI});
                            if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                                cout << "Push loop exit block B" << exitBlock.getCfgBlockID() << endl;
                            }
                        }
                    } else {
                        // Enter loop body
                        blockSuccs.push_back({bodyBlock, enterSI});
                        if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                            cout << "Push loop body block B" << bodyBlock.getCfgBlockID() << endl;
                        }
                    }
                        
                } else {
                    // Loop exit and enter both, check if exit block exists
                    if (exitBlock.getCfgBlock()) {
                        blockSuccs.push_back({exitBlock, exitSI});
                        if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                            cout << "Push loop exit block B" << exitBlock.getCfgBlockID() << endl;
                        }
                    }
                    if (bodyBlock.getCfgBlock()) {
                        // Body block can be @nullptr
                        blockSuccs.push_back({bodyBlock, enterSI});
                        if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                            cout << "Push loop body block B" << bodyBlock.getCfgBlockID() << endl;
                        }
                    }
                }
                
            } else
            if ( isa<const BreakStmt>(term) ) {
                
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    if (!deadCode) term->dumpColor();
                }
                SCT_TOOL_ASSERT (!loopStack.empty(),
                                 "Loop stack is empty in break");

                // Generate @break statement string
                auto& currLoop = loopStack.back();
                
                // One successor block
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 1, 
                                 "No one successor in break");
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "Break Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(), 
                                "No successor reachable in break");
                // Check if the next block is loop input Phi function and 
                // current block is loop body input/outside input
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);
 
                // Next block is outside of the loop, so it at least one level up
                // Use loop scope level @currLoop.second here 
                // Use after wait flag for removed loop as it run in separate analysis
                ConstScopeInfo si(state, currLoop.level, block, loopInputBody, 
                                  loopInputOut, loopStack, deadCode, 
                                  visitedLoops, currLoop.stmt);
                blockSuccs.push_back({succBlock, si});
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "    block level "<< si.level <<", flags " << loopInputBody << ", " << loopInputOut << endl;
                }
            } else
            if ( isa<const ContinueStmt>(term) ) {
                
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    if (!deadCode) term->dumpColor();
                }
                SCT_TOOL_ASSERT (!loopStack.empty(), 
                                 " Loop stack is empty in continue");
                // One successor block
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 1,
                                 "No one successor in continue");
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "Continue Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(), 
                                 "No successor reachable in continue");
                // Check if the next block is loop input Phi function and 
                // current block is loop body input/outside input
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);
 
                // Next block is inside of the loop, so it at least one level up
                // Use after wait flag for removed loop as it run in separate analysis
                ConstScopeInfo si(state, level, block, 
                                  loopInputBody, loopInputOut, 
                                  loopStack, deadCode, visitedLoops);
                blockSuccs.push_back({succBlock, si});
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "    block level "<< si.level << ", flags " << loopInputBody << ", " << loopInputOut << endl;
                }
            } else    
            if ( isa<const GotoStmt>(term) ) {
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
                stmtStoredValue.clear();
            }

        } else {
            // Block has one successor
            if (cfgBlock->succ_size() == 1) {
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(), "No reachable successor");

                // Check if the next block is loop input Phi function and 
                // current block is loop body input/outside input
                bool loopInputBody = checkLoopInputFromBody(block, succBlock);
                bool loopInputOut  = checkLoopInputFromOut(block, succBlock);

                // Level not changed in general block, level up provided
                // for next block if it has multiple inputs 
                ConstScopeInfo si(state, level, block,
                                  loopInputBody, loopInputOut, 
                                  loopStack, deadCode, visitedLoops);
                blockSuccs.push_back({succBlock, si});
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "    block level " << si.level << ", flags " << loopInputBody << ", " << loopInputOut << endl;
                }
                
            } else 
            if (cfgBlock->succ_size() > 1) {
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "Block " << cfgBlock->getBlockID() << " has " << block.getCfgBlock()->succ_size() << " successors" << endl;
                }
                SCT_TOOL_ASSERT (false, "Too many successors");

            } else {
                // Do nothing if no successor, exit block has no successors
                SCT_TOOL_ASSERT (cfgBlock->getBlockID() == exitBlockId,
                                 "Block with no successor is not exit block");
                // Any function has one exit block, which is analyzed last,
                // so @state contains the final state
                
                finalState = shared_ptr<ScState>(state->clone());
                    
                if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
                    cout << "----------------------------------------" << endl;
                    cout << " No more successors, exit from function "  << funcDecl->getNameAsString() << endl;
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
                    // Join scope info from the same predecessor 
                    auto& preds = i->second;
                    for (auto& j : preds) {
                        if (j.prevBlock == bs.second.prevBlock) {
                            found = true;
                            //cout << "Join states for prevBlock B#"<< j.prevBlock.getCfgBlockID() << endl;
                            j.state->join(bs.second.state.get());
                            break;
                        }
                    }
                      
                    if (!found) {
                        preds.push_back(bs.second);
                    }
                    
                    found = true;
                    // Join reachability by OR, 
                    // replace unreachable block with reachable
                    if (!i->first.isReachable() && bs.first.isReachable()) {
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
                delayed.insert(j, {bs.first, vector<ConstScopeInfo>(1, bs.second)});
                
                //cout << "Add to delayed new Block B" << AdjBlock::getCfgBlock(bs.first)->getBlockID() << endl;
                //cout << "delayed size is " << delayed.size() << endl;
            }
        }

        // Get next block from @delayed
        if (!delayed.empty()) {
            // Take block from @delayed
            auto i = delayed.rbegin();
            
            //cout << endl << "------- Choosing next block form delayed, size = " 
            //     << delayed.size() << endl;
            
            for (; i != delayed.rend(); ++i) {
                // Consider possible unreachable block
                unsigned predSize  = getPredsNumber(i->first);
                
                // Loop input from body/outside
                bool loopInputBody = i->second.begin()->loopInputBody;
                bool loopInputOut  = i->second.begin()->loopInputOut;
                
                //cout << "   B" << i->first.getCfgBlockID()
                //     << ", preds/inputs " << predSize << "/" << i->second.size()
                //     << ", flags " << loopInputBody << loopInputOut << endl;
                    
                // Check number of ready inputs equals to predecessor number
                // At loop input from body only one body predecessor ready
                // At loop input from outside body predecessor not ready
                if (i->second.size() >= predSize || 
                    (loopInputBody && i->second.size() >= 1) ||
                    (loopInputOut && i->second.size() >= predSize-1))
                {
                    // Number of ready inputs can exceed predecessor number as
                    // duplicate predecessor is possible from dead code in THREADs
                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                        cout << endl << "Next BLOCK B" << i->first.getCfgBlockID() 
                             << ", flags "<< loopInputBody << ", " << loopInputOut 
                            << ", ready inputs " << i->second.size() << ", preds " << predSize << endl;
                    }
                    
                    // Prepare next block
                    prepareNextBlock(i->first, i->second);
                    //state->print();
                    break;
                }
            }
            
            // Check there is ready block
            if (i == delayed.rend()) {
                cout << "Error in analyzed function " << funcDecl->getNameAsString() << endl;
                if (term) {
                    cout << term->getSourceRange().getBegin().printToString(sm) << endl;
                } else 
                if (currStmt) {
                    cout << currStmt->getSourceRange().getBegin().printToString(sm) << endl;
                }
                SCT_TOOL_ASSERT (false, "No ready block in @delayed");
            }

            // Remove block from @delayed, use ++ as @base() points to next object 
            delayed.erase((++i).base());

        } else {
            // No more blocks to analysis 
            if (contextStack.empty()) {
                //state->print();
                break;
            }
            
            // Restore callee function context
            restoreContext();
            skipOneElement = true;
        }
    }
}

// Run for function declaration, the same as @setFunction() and @run()
void ScTraverseConst::run(const clang::FunctionDecl* fdecl)
{
    setFunction(fdecl);
    run();
}

// Current thread has reset signal
void ScTraverseConst::setHasReset(bool hasReset_)
{
    hasReset = hasReset_;
}

}