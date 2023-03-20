/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "sc_tool/cfg/ScTraverseConst.h"
#include "sc_tool/expr/ScParseExprValue.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/elab/ScElabDatabase.h"
#include "sc_tool/elab/ScVerilogModule.h"
#include "sc_tool/elab/ScObjectView.h"
#include "sc_tool/utils/CppTypeTraits.h"

#include "clang/AST/Decl.h"
#include <memory>

namespace sc {

using namespace std;
using namespace clang;
using namespace llvm;

// ---------------------------------------------------------------------------
// Creates constant value in global state if necessary
// Used for constant and constant array belong to template parameter class
void ScTraverseConst::parseGlobalConstant(const SValue& val)
{
    //cout << "parseGlobalConstant val " << val << endl;
    if (val.getVariable().getParent() || 
        elabDB == nullptr || globalState == nullptr ) {
        // Do nothing
        
    } else
    if (!globalState->getElabObject(val)) {
        auto currentModule = *state->getElabObject(dynmodval);
        auto valDecl = const_cast<clang::ValueDecl*>(val.getVariable().getDecl());
        auto varDecl = dyn_cast<clang::VarDecl>(valDecl);

        // Create VerilogVar object in VerilogModule
        auto newElabObj = elabDB->createStaticVariable(currentModule, varDecl);
        verMod->addConstDataVariable(newElabObj, *(newElabObj.getFieldName()));

        // Parse and put initializer into global state, check no such value 
        // in state to avoid replace constant/static array elements
        if (!globalState->getValue(val)) {
            // Evaluate constant value from AST if possible to have clean UseDef 
            SValue rval;
            if (getConstASTValue(astCtx, val, rval)) {
                globalState->putValue(val, rval);
            } else {
                globExprEval.parseValueDecl(valDecl, NO_VALUE, nullptr, false);
            }
        }

        // Put the same initializer into current state, to use it in the process
        // For next analyzed processes it is taken from @globalState
        if (!state->getValue(val)) {
            // Evaluate constant value from AST if possible to have clean UseDef 
            SValue rval;
            if (getConstASTValue(astCtx, val, rval)) {
                state->putValue(val, rval);
            } else {
                parseValueDecl(valDecl, NO_VALUE, nullptr, false);
            }
        }
        
        globalState->putElabObject(val, newElabObj);
    }
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
                
                // Report error for any channel read in reset 
                QualType type = val.getType();
                // @sc_vector can contain channels only
                if (isScChannel(type, true) || isScChannelArray(type, true) || 
                    isScVector(type)) {
                    if (SValue var = state->getVariableForValue(val)) {
                        if (auto varDecl = var.getVariable().getDecl()) {
                            ScDiag::reportScDiag(varDecl->getBeginLoc(),
                                ScDiag::SC_CHAN_READ_IN_RESET) << var.asString(false);
                        } else {
                            ScDiag::reportScDiag(stmt->getBeginLoc(),
                                ScDiag::SC_CHAN_READ_IN_RESET) << val.asString(false);
                        }
                    } else {
                        ScDiag::reportScDiag(stmt->getBeginLoc(),
                            ScDiag::SC_CHAN_READ_IN_RESET) << val.asString(false);
                    }
                }
            }
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

// ---------------------------------------------------------------------------
// Auxiliary functions

// Evaluate terminator condition if it is compile time constant
void ScTraverseConst::evaluateTermCond(Stmt* stmt, SValue& val) 
{
    // Initialization section FOR loop analyzed before the FOR terminator 
    // Get condition for terminator, LSH for binary &&/||
    auto cond = getTermCond(stmt);
    
    // Consider only integer value in condition as pointer is casted to bool
    if (cond) {
        // Check if loop condition does not have compound statement or comma
        if (isa<ForStmt>(stmt) || isa<WhileStmt>(stmt) || isa<DoStmt>(stmt)) {
            bool error = false;
            if (isa<CompoundStmt>(cond)) error = true;
            if (auto binstmt = dyn_cast<const BinaryOperator>(cond)) {
                BinaryOperatorKind opcode = binstmt->getOpcode();
                if (opcode == BO_Comma) error = true;
            }

            if (error) {
                ScDiag::reportScDiag(cond->getBeginLoc(),
                                     ScDiag::CPP_LOOP_COMPOUND_COND);
            }
        }

        //cout << "Condition #" << hex << stmt << dec << endl;
        //cond->dumpColor();
        
        // Use @checkRecOnly to have pointer null/not null value for record/MIF
        // array element accessed at unknown index, required for condition of
        // pointer initialized: if (p) p->f(); p ? p->f() : a;
        auto condvals = evaluateConstInt(const_cast<Expr*>(cond), false, true);
        //cout << "evaluateConstInt " << condvals.first << " " << condvals.second << endl;
        readFromValue(condvals.first);
        val = condvals.second;
        
        // Store condition to avoid double parsing, required for &&/|| and ?
        if (isa<ConditionalOperator>(stmt) || isa<BinaryOperator>(stmt)) {
            auto i = condStoredValue.emplace(stmt, condvals.first);
            if (!i.second) {
                i.first->second = condvals.first;
            }
            //cout << "condStoredValue " << hex << stmt << " " << condvals.first << dec << endl;
        }
    }
}

// Evaluate literal or constant expression as non-negative integer
llvm::Optional<unsigned> ScTraverseConst::evaluateConstExpr(Expr* expr) 
{
    SValue rval;
    if (auto intLiter = dyn_cast<IntegerLiteral>(expr)) {
        ScParseExpr::parseExpr(intLiter, rval);
    } else 
    if (expr->getType().isConstQualified()) {
        rval = evaluateConstInt(expr, true).second;
    }
    
    if (rval.isInteger()) {
        return rval.getInteger().getZExtValue(); 
    }
    return llvm::None;
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
                if (auto inum = evaluateConstExpr(binoper->getRHS())) {
                    return *inum;
                } else 
                if (auto inum = evaluateConstExpr(binoper->getLHS())) {
                    return *inum;
                }
            }
        }
    }
    return llvm::None;
}

// Store ternary statement condition for SVA property
void ScTraverseConst::putSvaCondTerm(const Stmt* stmt, SValue val) 
{
    auto callStack = contextStack.getStmtStack();
    callStack.push_back(stmt);
    // Add the same value for first iteration
    callStack.push_back(stmt);

    auto i = termConds.emplace(callStack, val);
    if (!i.second) {
        if (i.first->second != val) {
            i.first->second = NO_VALUE;
            val = NO_VALUE;
        }
    }
    
    //cout << "putSvaCondTerm " << hex << stmt << " stackSize " << callStack.size()  
    //     << " val " << val << dec << endl;
}

// Prepare next block analysis
void ScTraverseConst::prepareNextBlock(AdjBlock& nextBlock, 
                                       vector<ConstScopeInfo>& scopeInfos) 
{
    // Taken block must have any predecessor
    SCT_TOOL_ASSERT (!scopeInfos.empty(), "No predecessor for next block");
    
    // Set previous and new current block
    prevBlock = block;
    block = nextBlock;
    
    // Loop stacks should be same from all inputs
    loopStack = scopeInfos.front().loopStack;
    for (const auto& si : scopeInfos) {
        // This error happens for return in loop, use not fatal error to 
        // report it correctly in TraverseProc
        if (si.loopStack != loopStack) {
            SCT_INTERNAL_ERROR_NOLOC ("Different loop stacks in predecessors");
        }
    }
    
    // Join visited loops by OR
    if (!isCombProcess) {
        visitedLoops.clear();
        for (const auto& si : scopeInfos) {
            visitedLoops.insert(si.visitedLoops.begin(), 
                                si.visitedLoops.end());
        }
    }
    
    // Join dead code flags by AND form all inputs 
    deadCond = true;
    for (const auto& si : scopeInfos) {
        deadCond = deadCond && si.deadCond;
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

    SCT_TOOL_ASSERT (!inFuncParams, 
                     "Function call or constructor in another call parameter");
    
    // Store return value for the call expression to replace where it used
    auto i = calledFuncs.emplace(expr, retVal);
    if (!i.second) {
        cout << hex << expr << dec << endl;
        SCT_TOOL_ASSERT (false, "Second meet of function call");
    }

    // Prepare context to store
    lastContext = std::make_shared<ConstFuncContext>(
                            CfgCursor(funcDecl, nullptr, 0), 
                            returnValue, modval, recval,
                            delayed, loopStack, calledFuncs,
                            simpleReturnFunc, returnStmtFunc, sideEffectFunc);
    
    // Set module, dynamic module and return value for called function
    modval = funcModval;
    recval = funcRecval;
    returnValue = retVal;
    funcDecl = callFuncDecl;
    simpleReturnFunc = true;
    returnStmtFunc = nullptr;
    sideEffectFunc = false;
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

void ScTraverseConst::parseReturnStmt(ReturnStmt* stmt, SValue& val)
{
    ScParseExprValue::parseReturnStmt(stmt, val);

    // Empty call stack possible for return from process function
    auto callStack = contextStack.getStmtStack();
    if (!callStack.empty()) {
        // Try to get integer value for return value assignment
        SValue rval = getValueFromState(val);

        if (rval.isInteger()) {
            auto i = constEvalFuncs.emplace(callStack, rval);
            if (!i.second) {
                if (i.first->second != rval) {
                    i.first->second = NO_VALUE;
                }
            }
        } else {
            constEvalFuncs[callStack] = NO_VALUE;
        }
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
    if (!funcDecl) {
        ScDiag::reportScDiag(expr->getBeginLoc(),
                             ScDiag::SYNTH_INCORRECT_FUNC_CALL) << "---";
    }
    
    string fname = funcDecl->getNameAsString();
    auto nsname = getNamespaceAsStr(funcDecl);

    if (fname == "__assert" || fname == "__assert_fail") {
        // Do nothing, implemented in ScParseExprValue
        
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
            ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::CPP_ASSERT_FAILED);
            SCT_TOOL_ASSERT (false, "Incorrect level stop");
        }
        val = NO_VALUE; // No value returned from assert

    } else
    if (nsname && *nsname == "sc_core") {
        if (fname == "wait") {
            waitCall = parseWaitArg(expr);
        } else {
           // Do nothing, implemented in ScParseExprValue 
        }
    } else     
    if (nsname && *nsname == "sc_dt") {
        // Do nothing, implemented in ScParseExprValue
        
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
        // No user-define function call in constant evaluation mode
        if (evaluateConstMode) {
            return;
        }
        
        if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for FUNCTION : " << fname << " (" 
                 << expr->getSourceRange().getBegin().printToString(sm) << ") |" << endl;
            cout << "-------------------------------------" << endl;
        }
        
        if (!isUserCallExpr(expr)) {
            ScDiag::reportScDiag(expr->getBeginLoc(),
                                 ScDiag::SYNTH_INCORRECT_FUNC_CALL) << fname;
        }

        if (nsname && *nsname == "std") {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::CPP_UNKNOWN_STD_FUNC) << fname;
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
void ScTraverseConst::parseMemberCall(CXXMemberCallExpr* expr, SValue& tval, 
                                      SValue& val) 
{
    // Get @this expression and its type
    Expr* thisExpr = expr->getImplicitObjectArgument();
    QualType thisType = thisExpr->getType();
    bool isZeroWidth = isZeroWidthType(thisType);
    
    // @this value for user function
    ScParseExprValue::parseMemberCall(expr, tval, val);
    // Return value passed in @val
    SValue retVal = val;
        
    // Get method
    FunctionDecl* methodDecl = expr->getMethodDecl()->getAsFunction();
    string fname = methodDecl->getNameAsString();
    
    if ( isAnyScIntegerRef(thisType, true) || isZeroWidth ) {
        // Do nothing, all logic implemented in ScParseExprValue
        
    } else 
    if ( isScChannel(thisType) ) {
        // Do nothing, all logic implemented in ScParseExprValue
        
    } else
    if ( isAnyScCoreObject(thisType) ) {
        if (fname == "wait" ) {
            // SC wait call
            waitCall = parseWaitArg(expr);
            
        } else {
            // Do nothing for other @sc_core methods
        }
        
    } else
    if (isConstCharPtr(expr->getType())) {
        // Do nothing, all logic implemented in ScParseExprValue
        
    } else  
    if (state->getParseSvaArg()) {
        // Do nothing for function call in SVA, all done in ScParseExprValue
        
    } else {
        // General method call
        // No user-define method call in constant evaluation mode
        if (evaluateConstMode) {
            return;
        }
        
        if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for METHOD FUNCTION : " << fname << " (" 
                 << expr->getBeginLoc().printToString(sm) 
                 << " retVal " << retVal << ") |" << endl;
            cout << "-------------------------------------" << endl;
        }
        
        if (!isUserCallExpr(expr)) {
            ScDiag::reportScDiag(expr->getBeginLoc(),
                                 ScDiag::SYNTH_INCORRECT_FUNC_CALL) << fname;
        }

        // Get record from variable/dynamic object
        SValue ttval = getRecordFromState(tval, ArrayUnkwnMode::amFirstElement);
        
        // Allowed parent kinds
        if (!ttval.isArray() && !ttval.isRecord() && !ttval.isVariable()) {
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(),
                                 ScDiag::SYNTH_INCORRECT_RECORD) 
                            << tval.asString() << ttval.asString();
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
        if (methodDecl->isVirtualAsWritten() && !hasClassCast) {
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
            auto virtPair = getVirtualFunc(dyntval, methodDecl);
            funcModval = virtPair.first;
            methodDecl = virtPair.second;
        }

        // Check function is not pure
        if (methodDecl->isPure()) {
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
        prepareCallParams(expr, fval, methodDecl);
        // Register return value and prepare @lastContext
        prepareCallContext(expr, fval, NO_VALUE, methodDecl, retVal);
        // Return value variable has call point level
        state->setValueLevel(retVal, level);
    }
}

// Operator call expression0
void ScTraverseConst::parseOperatorCall(CXXOperatorCallExpr* expr, SValue& tval,
                                        SValue& val) 
{
    SCT_TOOL_ASSERT (expr->getNumArgs() != 0, "Operator without arguments");
    Expr* thisExpr = expr->getArgs()[0];
    bool isZeroWidth = isZeroWidthType(thisExpr->getType());

    // @this value for user function
    ScParseExprValue::parseOperatorCall(expr, tval, val);
        
    // Get operator method
    FunctionDecl* methodDecl = expr->getCalleeDecl()->getAsFunction();
    string fname = methodDecl->getNameAsString();
    
    OverloadedOperatorKind opcode = expr->getOperator();
    bool isAssignOperator = expr->isAssignmentOp() && opcode == OO_Equal;
    bool isSctChan = isAssignOperatorSupported(tval.getType());
    
    if (isZeroWidth) {
        // Do nothing 
        
    } else 
    if (isAssignOperator && isSctChan) {
        // Operator call in sct namespace
        // No user-define method call in constant evaluation mode
        if (evaluateConstMode) {
            return;
        }
        
        // Return value passed in @val
        SValue retVal = val;
        
        if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for OPERATOR : " << fname << " (" 
                 << expr->getBeginLoc().printToString(sm) 
                 << " retVal " << retVal << ") |" << endl;
            cout << "-------------------------------------" << endl;
        }
        
        // Get record from variable/dynamic object
        SValue ttval = getRecordFromState(tval, ArrayUnkwnMode::amFirstElement);
        
        // Allowed parent kinds
        if (!ttval.isArray() && !ttval.isRecord() && !ttval.isVariable()) {
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(),
                                 ScDiag::SYNTH_INCORRECT_RECORD) 
                            << tval.asString() << ttval.asString();
            ttval = NO_VALUE;
        }

        // Get dynamic class for member record
        SValue dyntval = ttval; 
        // @modval for called function analysis
        SValue funcModval = ttval;

        // Virtual operators not supported
        if (methodDecl->isVirtualAsWritten()) {
            SCT_INTERNAL_ERROR(expr->getBeginLoc(), "No virtual operator supported");
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
        prepareCallParams(expr, fval, methodDecl);
        // Register return value and prepare @lastContext
        prepareCallContext(expr, fval, NO_VALUE, methodDecl, retVal);
        // Return value variable has call point level
        state->setValueLevel(retVal, level);
    }
}

// Choose and run DFS step in accordance with expression type.
// Remove sub-statements from generator
void ScTraverseConst::chooseExprMethod(Stmt* stmt, SValue& val)
{
    //cout << "--- chooseExprMethod " << hex << stmt << dec << endl;
    // Constructor call for local record considered as function call
    bool anyFuncCall = isa<CallExpr>(stmt) || isa<CXXConstructExpr>(stmt);
    
    if (anyFuncCall && calledFuncs.count(stmt)) {
        // Get return value from already analyzed user defined functions
        val = calledFuncs.at(stmt);
        //cout << "Get RET value " << val << " for stmt #" << hex << stmt << dec  << endl;
        
    } else {
        // Normal statement analysis
        ScParseExpr::chooseExprMethod(stmt, val);
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
    SCT_TOOL_ASSERT (termConds.empty(), "@termCondResults is not empty");
    SCT_TOOL_ASSERT (liveStmts.empty(), "@liveStmts is not empty");
    SCT_TOOL_ASSERT (liveTerms.empty(), "@liveTerms is not empty");
    SCT_TOOL_ASSERT (funcDecl, "Function declaration and context stack not set");
    SCT_TOOL_ASSERT (constEvalFuncs.empty(), "@constEvalFuncs is not empty");

    cfg = cfgFabric->get(funcDecl);
    block = AdjBlock(&cfg->getEntry(), true);
    prevBlock = block;
    level = 0;
    elemIndx = 0;
    exitBlockId = cfg->getExit().getBlockID();
    deadCond = false;
    funcCall = false;
    aliveLoop = false;
    mainLoopStmt = nullptr;
    hasCodeBeforeMainLoop = false;
    simpleReturnFunc = false;       // Process function is not considered here
    returnStmtFunc = nullptr;
    sideEffectFunc = false;
    
    // Setup first non-MIF module value 
    synmodval = state->getSynthModuleValue(modval, ScState::MIF_CROSS_NUM);

    // Check if current module if element of array of MIF
    zeroElmtMIF = false;
    nonZeroElmtMIF = false;
    if (isScModularInterface(modval.getType())) {
        // Get all MIF arrays up to the parent module
        auto mifarrs = state->getAllMifArrays(modval, ScState::MIF_CROSS_NUM);
        
        mifElmtSuffix = "";
        for (const SValue& val : mifarrs) {
            SCT_TOOL_ASSERT (val.isArray() && !val.getArray().isUnknown(), 
                             "Unknown index for MIF array element");
            auto i = val.getArray().getOffset();
            nonZeroElmtMIF = nonZeroElmtMIF || i != 0;
            mifElmtSuffix += "["+ to_string(i) +"]";
        }
        
        if (!mifarrs.empty()) {
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
    delayed = context.delayed;
    simpleReturnFunc = context.simpleReturnFunc;
    returnStmtFunc = context.returnStmtFunc;
    // Side effects in called functions spread to callee
    sideEffectFunc = context.sideEffectFunc || sideEffectFunc;
        
    cfg = cfgFabric->get(funcDecl);
    SCT_TOOL_ASSERT (cfg, "No CFG at restore context");
    block = AdjBlock(const_cast<CFGBlock*>(context.callPoint.getBlock()), true);
    prevBlock = block;
    
    // Start analysis with restored element index
    elemIndx = context.callPoint.getElementID();
    exitBlockId = cfg->getExit().getBlockID();

    // Erase local variables of called function 
    state->removeValuesByLevel(level);
    
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

// ------------------------------------------------------------------------
// Main functions

// Remove unused variable definition statements in METHODs and CTHREADs
void ScTraverseConst::removeUnusedStmt() 
{
//    cout << "Simple statements: " << endl;
//    for (Stmt* stmt : simpleStmts) {
//        cout << "  " << hex << stmt << dec << endl;
//    } 
    
    while (true) {
        unordered_set<Stmt*> removeStmts;

        // Find defined but not used variables to remove
        //cout << "Unused val " << endl;
        for (auto i = defVarStmts.begin(); i != defVarStmts.end(); ) {
            const QualType& type = i->first.getType();
            // Skip pointer, channel and record values
            if (sc::isPointer(type) || 
                sc::isScChannel(type) || 
                sc::isScChannelArray(type) ||
                sc::isUserClass(getDerefType(type), true) ||
                sc::isUserDefinedClassArray(getDerefType(type), true)) 
            {++i; continue;}
            
            // Skip used value
            if (useVarStmts.count(i->first)) {++i; continue;}

            //cout << "  " << i->first << endl;
            for (Stmt* stmt : i->second) {
                if (simpleStmts.count(stmt)) {
                    removeStmts.insert(stmt);
                }
            }
            i = defVarStmts.erase(i);
        }
        // Stop if no statements removed
        if (removeStmts.empty()) break;

        // Remove unused statements from @useVarStmts
        for (auto i = useVarStmts.begin(); i != useVarStmts.end(); ) {
            for (Stmt* stmt : removeStmts) {
                i->second.erase(stmt);
            }
            if (i->second.empty()) {
                i = useVarStmts.erase(i);
            } else {
                ++i;
            }
        }

        // Remove unused statements from @liveStmts
        //cout << "Removed statements: " << endl;
        for (Stmt* stmt : removeStmts) {
            liveStmts.erase(stmt);
            //cout << "  " << hex << stmt << dec << endl;
        }
    }
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
    
    // Fill statement levels for entry function
    stmtInfo.run(funcDecl, 0);
    //stmtInfo.printLevels();
    //stmtInfo.printBreaks();

    while (true)
    {
        currStmt = nullptr;
        
        // Do not analyze statement for dead condition blocks, required for
        // complex conditions with side effects
        if (!deadCond) {
            if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                cout << "Analyze Block B" << block.getCfgBlockID() << endl;
            }            
            //cout << "elemId = " << elemIndx << ", block size = " << block.getCfgBlock()->size() << endl;
            SCT_TOOL_ASSERT (elemIndx <= block.getCfgBlock()->size(),
                            "Incorrect elmId");

            // Update loop stack for do...while loop enter
            if (auto doterm = getDoWhileTerm(block)) {
                // Check for CTHREAD main loop DO..WHILE
                // Set main loop as last high level loop with wait`s
                if (!isCombProcess && loopStack.empty() && contextStack.empty()) {
                    if (findWaitInLoop && findWaitInLoop->hasWaitCall(doterm)) {
                        mainLoopStmt = doterm;
                        //cout << "mainLoop " << endl; mainLoopStmt->dumpColor();
                    }
                }
                
                // Determine loop already visited with checking if 
                // current (last) loop has the same terminator
                bool loopVisited = loopStack.isCurrLoop(doterm);
                
//                if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
//                    cout << "--- DO term #" << hex << doterm << " : " 
//                         << (loopStack.size() ? loopStack.back().stmt : 0) 
//                         << dec <<" (" << loopStack.size() << ")" 
//                         << ", loopVisited " << loopVisited << endl;
//                }

                if (!loopVisited) {
                    unsigned iterNumber = 0;
                    if (auto in = evaluateIterNumber(doterm)) {
                        iterNumber = in.getValue();
                    }
                    // Add current loop into loop stack, set 1st iteration
                    loopStack.pushLoop({doterm, state->clone(), iterNumber, 1U});
                    
//                    if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
//                        cout << "Push loop stack(1) " << hex << doterm << dec 
//                             << ", level " << level << ", LS size " << loopStack.size() << endl;
//                    }
                }
                
                if (aliveLoop) {
                    ScDiag::reportScDiag(doterm->getBeginLoc(),
                                         ScDiag::SYNTH_ALIVE_LOOP_ERROR);
                }
                aliveLoop = false;
            }
            
            // CFG block body analysis, preformed for not dead state only
            for (unsigned i = elemIndx; i < block.getCfgBlock()->size(); ++i)
            {
                const CFGElement& elm = block.getCfgBlock()->operator [](i);
                SCT_TOOL_ASSERT (elm.getKind() == CFGElement::Kind::Statement,
                                 "Incorrect element kind");
                
                // Get statement 
                CFGStmt cfgstmt = elm.getAs<CFGStmt>().getValue();
                currStmt = const_cast<Stmt*>(cfgstmt.getStmt());
                //cout << "Stmt #" << hex << currStmt << dec << endl;
                //currStmt->dumpColor();
                
                // Get statement level and check if it is sub-statement
                bool isStmt = false; bool upLevel = false; 
                if (auto stmtLevel = stmtInfo.getLevel(currStmt)) {
                    upLevel = *stmtLevel < level;
                    level = *stmtLevel; 
                    isStmt = true;

                } else 
                if (auto stmtLevel = stmtInfo.getDeclGroupLevel(currStmt)) {
                    upLevel = *stmtLevel < level;
                    level = *stmtLevel;
                    isStmt = true;
                    
                } else 
                if (auto stmtLevel = stmtInfo.getSubStmtLevel(currStmt)) {
                    upLevel = *stmtLevel < level;
                    level = *stmtLevel;
                    isStmt = isUserCallExpr(currStmt);

                } else {
                    cout << hex << "#" << currStmt << dec << endl;
                    currStmt->dumpColor();
                    SCT_INTERNAL_ERROR(currStmt->getBeginLoc(), 
                                       "No level found for sub-statement");
                }
                // Skip sub-statement
                if (!isStmt) continue;
                
                // Erase local variables in Phi functions
                if (upLevel) {
                    state->removeValuesByLevel(level);
                }            
                
                // If started with restored context, move to the next element, 
                // element stored in context was already analyzed
                if (skipOneElement) {
                    skipOneElement = false;
                    continue;
                }

                if (DebugOptions::isEnabled(DebugComponent::doConstStmt)) {
                    cout << endl;
                    currStmt->dumpColor();
                    cout << "level " << level << endl;
                    //state->print();
                }

                // Parse statement and fill state
                isRequiredStmt = false;
                SValue val;
                chooseExprMethod(currStmt, val);
                SCT_TOOL_ASSERT (!assignLHS, "Incorrect assign LHS flag");

                // Register not mandatory required statements
                if (!isRequiredStmt) {
                    simpleStmts.insert(currStmt);
                }
                
                if (!mainLoopStmt && !isCombProcess) {
                    hasCodeBeforeMainLoop = true;
                }
                
                // Register statement as live
                liveStmts.insert(currStmt);
                
                //state->print();

                // Wait call, store state and continue analysis
                if (waitCall > 0 && cthreadStates) {
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
                    
                } else 
                if (waitCall > 0 && !cthreadStates) {
                    ScDiag::reportScDiag(currStmt->getSourceRange().getBegin(), 
                                         ScDiag::SC_WAIT_IN_METHOD);
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
                        SCT_INTERNAL_FATAL(currStmt->getBeginLoc(), 
                            "No function body found, probably STL function");
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

                    // Fill statement levels for current function
                    stmtInfo.run(funcDecl, level+1);  
                    // Increase level for called function
                    level += 1;

                    // Start called function with empty @delayed and loop stack
                    delayed.clear();
                    loopStack.clear();
                    calledFuncs.clear();

                    lastContext = nullptr;
                    funcCall = true;
                    break;
                }
            }
        }

        // Start any block expect return to callee with first element
        elemIndx = 0;
        
        // Block successors
        vector<pair<AdjBlock, ConstScopeInfo> >  blockSuccs;
        CFGBlock* cfgBlock = block.getCfgBlock();
        Stmt* term = (cfgBlock->getTerminator().getStmt()) ? 
                            cfgBlock->getTerminator().getStmt() : nullptr;
        
        if (funcCall) {
            // Suspend analysis of this function and go to called function
            funcCall = false;
            continue;
            
        } else 
        if (term) {
            // FOR/WHILE loop first iteration, used to get separate condition value
            bool loopTerm = isa<ForStmt>(term) || isa<WhileStmt>(term);
            bool loopFirstIter = loopTerm && !loopStack.isCurrLoop(term);
            //if (isa<ForStmt>(term) || isa<WhileStmt>(term) || isa<DoStmt>(term)) {
            //    cout << "loopFirstIter " << loopFirstIter << endl;
            //}
            
            // Set level for loop terminator
            if (isa<ForStmt>(term) || isa<WhileStmt>(term) || isa<DoStmt>(term)) {
                if (auto termLevel = stmtInfo.getLevel(term)) {
                    level = *termLevel;
                    //cout << "Term #" << term << " level " << level << endl;
                } else {
                    SCT_INTERNAL_ERROR(currStmt->getBeginLoc(), 
                                       "No level found for loop terminator");
                }
            }
            
            // Evaluate for-loop initializer and increment
            // FOR loop counter has terminator level, so not removed after loop
            if (ForStmt* forstmt = dyn_cast<ForStmt>(term)) {
                if (loopFirstIter) {
                    // At first iteration evaluate for-loop initializer and
                    // register internal counter variable
                    if (Stmt* init = forstmt->getInit()) {
                        SValue val;
                        chooseExprMethod(init, val);
                        
                        if (auto varDecl = ForLoopVisitor::get().
                                           getCounterDecl(forstmt)) {
                            val = SValue(varDecl, modval); 
                            state->regForLoopCounter(val);
                        }
                    }
                } else {
                    // At other iterations evaluate for-loop increment
                    if (Expr* inc = forstmt->getInc()) {
                        SValue val;
                        chooseExprMethod(inc, val);
                    }
                }
                
                if (Stmt* init = forstmt->getInit()) {
                    if (!isa<DeclStmt>(init)) {
                        ScDiag::reportScDiag(term->getBeginLoc(), 
                                             ScDiag::CPP_FOR_WITHOUT_DECL);
                    }
                }
            }

            // Try to calculate condition 
            isAssignStmt = false; isSideEffStmt = false;
            SValue termCondValue;
            evaluateTermCond(term, termCondValue);
            
            if (isAssignStmt) {
                ScDiag::reportScDiag(term->getBeginLoc(), 
                                     ScDiag::SYNTH_ASSIGN_IN_COND);
            }
            if (isSideEffStmt) {
                ScDiag::reportScDiag(term->getBeginLoc(), 
                                     ScDiag::SYNTH_SIDE_EFFECT_IN_COND);
            }
            
            // Check and apply alive loop macro to FOR/WHILE first iteration
            if (aliveLoop) {
                if (loopFirstIter) {
                    if (termCondValue.isInteger() && 
                        termCondValue.getInteger().isNullValue()) {
                        ScDiag::reportScDiag(term->getBeginLoc(),
                                             ScDiag::SYNTH_ALIVE_LOOP_NULL_COND);
                    }
                    termCondValue = SValue(SValue::boolToAPSInt(true), 10);
                    
                } else 
                if (!loopTerm) {
                    ScDiag::reportScDiag(term->getBeginLoc(),
                                         ScDiag::SYNTH_ALIVE_LOOP_ERROR);
                }
            }
            aliveLoop = false;
            
            bool trueCond = termCondValue.isInteger() && 
                             !termCondValue.getInteger().isNullValue();
            bool falseCond = termCondValue.isInteger() && 
                             termCondValue.getInteger().isNullValue();

            if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                //state->print();
                cout << "termCondValue " << termCondValue << endl;
            }
            
            // Store terminator condition to use in ScTraverseProc, 
            // different results joined to NO_VALUE
            auto callStack = contextStack.getStmtStack();
            callStack.push_back(term);
            // Use call stack with double #term to distinguish first iteration
            if (loopFirstIter) {
                callStack.push_back(term);
            }

            auto i = termConds.emplace(callStack, termCondValue);
            if (!i.second) {
                SValue& curVal = i.first->second;
                if (curVal.isInteger() && termCondValue.isInteger()) {
                    // Compare integer values only, no width/signess considered 
                    if (!APSInt::isSameValue(curVal.getInteger(), 
                                             termCondValue.getInteger())) {
                        curVal = NO_VALUE;
                    }
                } else {
                    if (curVal != termCondValue) {
                        curVal = NO_VALUE;
                    }
                }
            }

            //cout << "putTermConds " << hex << term << " stackSize " << callStack.size()  
            //     << " val " << termCondValue << dec << endl;
            
            // Register terminator as live
            liveTerms.insert(term);
            
            // Remove values at loop entry level
            if (isa<ForStmt>(term) || isa<WhileStmt>(term) || isa<DoStmt>(term)) {
                //term->dumpColor();
                //cout << "Remove value at term by level " << level << endl;
                state->removeValuesByLevel(level);
                
            } else {
                if (!mainLoopStmt && !isCombProcess) {
                    hasCodeBeforeMainLoop = true;
                }
            }
            
            // Analyze terminator
            if (isa<IfStmt>(term)) {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    term->dumpColor();
                }
                
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2, 
                                 "No two successors in IF");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock thenBlock(*iter);
                AdjBlock elseBlock(*(++iter));

                if (!falseCond) {
                    ConstScopeInfo thenES(state, block, loopStack, visitedLoops);
                    blockSuccs.push_back({thenBlock, thenES});
                }
                if (!trueCond) {
                    ConstScopeInfo elseES((falseCond) ? state : 
                                          shared_ptr<ScState>(state->clone()), 
                                          block, loopStack, visitedLoops);
                    blockSuccs.push_back({elseBlock, elseES});
                }

            } else
            if (SwitchStmt* swstmt = dyn_cast<SwitchStmt>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    term->dumpColor();
                }

                // Switch cases loop, skip first case as it belongs to @default
                const SwitchCase* swcase = swstmt->getSwitchCaseList();
                bool hasDefault = false;
                bool emptyDefault = false;
                SwitchCase* defcase = nullptr;
                
                if (swcase && isa<const DefaultStmt>(swcase) ) {
                    hasDefault = true;
                    emptyDefault = isDefaultEmpty(swcase);
                    defcase = const_cast<SwitchCase*>(swcase);
                    swcase = swcase->getNextSwitchCase();
                }
                        
                // Cases with their blocks, reordered in direct order
                auto cases = getSwitchCaseBlocks(swcase, cfgBlock);
                
                bool allEmptyCases = true; // All cases including default are empty
                bool constCase = false; // One case chosen by constant condition
                bool prevEmptyCase = false; // Previous case is empty 
                bool clone = false; // Clone @state after first case
                
                // Cases loop
                for (auto entry : cases) {
                    SwitchCase* swcase = const_cast<SwitchCase*>(entry.first);
                    AdjBlock caseBlock = entry.second;
                    SCT_TOOL_ASSERT (caseBlock.getCfgBlock(), "No switch case block");
                    
                    bool emptyCase = isCaseEmpty(caseBlock);
                    allEmptyCases = allEmptyCases && emptyCase;
                    
                    SValue caseValue;
                    if (CaseStmt* cstmt = dyn_cast<CaseStmt>(swcase)) {
                        // Get case expression and store it in generator
                        Expr* expr = cstmt->getLHS();
                        // Evaluate case value
                        caseValue = evaluateConstInt(expr, false).second;
                        
                    } else {
                        SCT_TOOL_ASSERT (false, "Unexpected statement type in switch");
                    }
                    
                    bool deadCase = false;
                    if (termCondValue.isInteger() && caseValue.isInteger()) {
                        deadCase = !APSInt::isSameValue(termCondValue.getInteger(), 
                                                        caseValue.getInteger());
                        constCase = constCase || !deadCase;
                    }
                    
                    if (!deadCase) {
                        ConstScopeInfo caseSI(
                            (clone) ? shared_ptr<ScState>(state->clone()) : state, 
                            block, loopStack, visitedLoops);
                        blockSuccs.push_back({caseBlock, caseSI});
                        clone = true;

                        if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                            cout << "CaseBlock B" << caseBlock.getCfgBlockID() << endl;
                        }
                    }
                    
                    // Register live case
                    if (!deadCase || prevEmptyCase) {
                        liveTerms.insert(swcase);
                        prevEmptyCase = emptyCase;
                    } else {
                        prevEmptyCase = false;
                    }
                }

                // Last successor block is default or after switch block
                AdjBlock succBlock(*cfgBlock->succ_rbegin());
                
                // Report all empty cases, not supported in @calcNextLevel()
                allEmptyCases = allEmptyCases && hasDefault && emptyDefault;
                if (allEmptyCases) {
                    ScDiag::reportScDiag(swstmt->getBeginLoc(),
                                         ScDiag::SYNTH_SWITCH_ALL_EMPTY_CASE);
                    // To avoid further errors
                    SCT_INTERNAL_FATAL_NOLOC ("Empty switch without break not supported");
                }
                
                bool deadDefault = hasDefault && 
                                   (constCase || !succBlock.isReachable());
                if (!deadDefault) {
                    ConstScopeInfo switchSI(
                        (clone) ? shared_ptr<ScState>(state->clone()) : state, 
                        block, loopStack, visitedLoops);
                    blockSuccs.push_back({succBlock, switchSI});
                    
                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                        cout << "Switch Default B" << succBlock.getCfgBlockID() << endl;
                    }
                }

                // Register live default
                if (!deadDefault || prevEmptyCase) {
                    liveTerms.insert(const_cast<SwitchCase*>(defcase));
                }
            } else 
            if (BinaryOperator* binstmt = dyn_cast<BinaryOperator>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    term->dumpColor();
                }

                BinaryOperatorKind opcode = binstmt->getOpcode();
                SCT_TOOL_ASSERT (opcode == BO_LOr || opcode == BO_LAnd, 
                                 "Incorrect terminator statement");

                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in ||/&&");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock thenBlock(*iter);
                AdjBlock elseBlock(*(++iter));
                
                // Only path to final IF/Loop/? statement is taken, 
                // if this path is dead it is detected at the final statement
                // @deadThen/Else not consider @deadCond required for complex conditions
                if (opcode == BO_LOr) {
                    ConstScopeInfo elseSI(state, block,
                                          loopStack, visitedLoops, trueCond);
                    blockSuccs.push_back({elseBlock, elseSI});
                }
                if (opcode == BO_LAnd) {
                    ConstScopeInfo thenSI(state, block,
                                          loopStack, visitedLoops, falseCond);
                    blockSuccs.push_back({thenBlock, thenSI});
                }
                
            } else
            if ( isa<ConditionalOperator>(term) )
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    term->dumpColor();
                }

                // Two argument blocks    
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in cond operator");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock argBlock1(*iter);
                AdjBlock argBlock2(*(++iter));
                
                if (!falseCond) {
                    ConstScopeInfo argSI1(state, block, loopStack, visitedLoops);
                    blockSuccs.push_back({argBlock1, argSI1});
                }
                if (!trueCond) {
                    ConstScopeInfo argSI2((falseCond) ? state : 
                                           shared_ptr<ScState>(state->clone()), 
                                           block, loopStack, visitedLoops);
                    blockSuccs.push_back({argBlock2, argSI2});
                }

            } else                 
            if (isa<ForStmt>(term) || isa<WhileStmt>(term) || isa<DoStmt>(term))
            {
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    term->dumpColor();
                }
                //cout << "Loop "<< hex << term << dec << " termCondValue " << termCondValue << endl;
                //state->print();
                
                // Check for CTHREAD main loop WHILE/FOR
                // Set main loop as last high level loop with wait`s
                if (!isCombProcess && loopStack.empty() && contextStack.empty()) {
                    if (findWaitInLoop && findWaitInLoop->hasWaitCall(term)) {
                        mainLoopStmt = term;
                        //cout << "mainLoop :"; mainLoopStmt->getBeginLoc().dump(sm);
                        //cout << endl;
                    }
                }
                
                if (!mainLoopStmt && !isCombProcess) {
                    hasCodeBeforeMainLoop = true;
                }
                
                // Determine loop already visited with checking if 
                // current (last) loop has the same terminator
                bool loopVisited = loopStack.isCurrLoop(term);
                
//                if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
//                    cout << "--- Loop term #" << hex << term << " : " 
//                         << (loopStack.size() ? loopStack.back().stmt : 0) 
//                         << dec <<" (" << loopStack.size() << ")" << endl;
//                }
                
                if (!isCombProcess) {
                    // Checking for fall through path in this loop
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

                // Stop loop analysis if state is not changed at last iteration
                bool stableState = false;
                
                if (loopVisited) {
                    // Maximal iteration number to analyze
                    unsigned loopMaxIter = (loopStack.size() <= DEEP_LOOP_DEPTH) ? 
                                            LOOP_MAX_ITER : DEEP_LOOP_MAX_ITER;
                    // Iteration number or zero
                    unsigned iterNumber = loopStack.back().iterNumber;
                    // Loop with fixed and quite small iteration number
                    bool fixedLoop = iterNumber && iterNumber < loopMaxIter-1;
                    // Counter of already analyzed iterations
                    unsigned iterCntr = loopStack.back().counter;
                    
                    // Check if state changed at some analysis iterations
                    // For fixed loop it is last iteration and last max iteration
                    bool compareState = (iterCntr >= loopMaxIter-1) ||
                                        (fixedLoop ? iterCntr == iterNumber :
                                         iterCntr == COMPARE_STATE_ITER);
                     
                    if (compareState) {
                        stableState = state->compare(loopStack.back().state.get());
                        if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
                            cout << "Loop stable state " << stableState << endl;
                        }
                    }
                    
                    // Check and increment iteration counter
                    if (!stableState) {
                        if (iterCntr < loopMaxIter) {
                            // Normal iterations
                            if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
                                cout << "Loop next iteration " << hex << loopStack.back().stmt << dec 
                                     << ", counter " << iterCntr << endl;
                            }
                            loopStack.back().counter += 1;
                            // Store state for next comparison
                            if (compareState) {
                                loopStack.back().state = 
                                        shared_ptr<ScState>(state->clone());
                            }

                        } else 
                        if (iterCntr >= loopMaxIter && iterCntr < UNROLL_ERROR_ITER) 
                        {
                            // Several last iterations with replace to NO_VALUE,
                            // required to spread NO_VALUE through all conditions
                            if (DebugOptions::isEnabled(DebugComponent::doConstLoop)) {
                                cout << "Loop last iteration(s) " << hex << loopStack.back().stmt << dec 
                                     << ", counter " << iterCntr << endl;
                                    cout << "> Set NO_VALUE for unstable state delta" << endl;
                            }
                            loopStack.back().counter += 1;
                            
                            // Set NO_VALUE for all different tuples,
                            // at next iteration state should be stable
                            state->compareAndSetNovalue(loopStack.back().state.get());

                            // Store state for next comparison
                            loopStack.back().state = 
                                        shared_ptr<ScState>(state->clone());

                        } else {
                            ScDiag::reportScDiag(term->getBeginLoc(), 
                                                 ScDiag::SC_ERROR_CPROP_UNROLL_MAX);
                            // Set to exit from this loop
                            stableState = true;
                        }
                        loopStack.back().stableState = false;
                    
                    } else {
                        // If state is stable one more iteration required to ensure 
                        // it is really stable, but not eventual coincidence
                        if (loopStack.back().stableState) {
                            // Second iteration with stable state, loop finished

                        } else {
                            // First iteration with stable state, needs one more
                            if (iterCntr < loopMaxIter) {
                                loopStack.back().counter = loopMaxIter;
                            } else {
                                loopStack.back().counter += 1;
                            }
                            loopStack.back().stableState = true;
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
                    loopStack.pushLoop({term, state->clone(), iterNumber, 1U});
                }
                
                bool addBodyBlock = !stableState && bodyBlock.getCfgBlock();
                bool addExitBlock = (stableState || !trueCond) && 
                                    exitBlock.getCfgBlock();      
                bool clone = false;    
                if (addBodyBlock && !falseCond) {
                    ConstScopeInfo enterSI(state, block, loopStack, visitedLoops);
                    blockSuccs.push_back({bodyBlock, enterSI});
                    clone = true;
                    
//                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
//                        cout << "   Push loop body block B" << bodyBlock.getCfgBlockID() << endl;
//                    }
                }
                if (addExitBlock) {
                    visitedLoops.erase(loopStack.back().stmt);
                    loopStack.pop_back();
                    ConstScopeInfo exitSI(clone ? shared_ptr<ScState>(
                                          state->clone()) : state, 
                                          block, loopStack, visitedLoops);
                    blockSuccs.push_back({exitBlock, exitSI});
                    
//                    if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
//                        cout << "   Push loop exit block B" << exitBlock.getCfgBlockID() << endl;
//                    }
                }
                
            } else
            if ( isa<BreakStmt>(term) ) {
                
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    term->dumpColor();
                }
                
                bool isLoopBreak = !stmtInfo.isSwitchBreak(term);
                
                // Remove last loop from stack for loop break
                if (isLoopBreak) {
                    SCT_TOOL_ASSERT (!loopStack.empty(), 
                                     "Loop stack is empty in break");
                    if (!breakInRemovedLoop) {
                        bool waitInLoop = findWaitInLoop && 
                            findWaitInLoop->hasWaitCall(loopStack.back().stmt);
                        breakInRemovedLoop = waitInLoop;
                    }
                    visitedLoops.erase(loopStack.back().stmt);
                    loopStack.pop_back();
                }
                
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
                
                // Use after wait flag for removed loop as it run in separate analysis
                ConstScopeInfo si(state, block, loopStack, visitedLoops);
                blockSuccs.push_back({succBlock, si});

            } else
            if ( isa<ContinueStmt>(term) ) {
                
                if (DebugOptions::isEnabled(DebugComponent::doConstTerm)) {
                    term->dumpColor();
                }
                
                SCT_TOOL_ASSERT (!loopStack.empty(), 
                                 " Loop stack is empty in continue");
                if (!breakInRemovedLoop) {
                    bool waitInLoop = findWaitInLoop && 
                            findWaitInLoop->hasWaitCall(loopStack.back().stmt);
                    breakInRemovedLoop = waitInLoop;
                }
                
                // One successor block
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 1,
                                 "No one successor in continue");
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "Continue Succ Block B" << succBlock.getCfgBlockID() << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(), 
                                 "No successor reachable in continue");
 
                // Use after wait flag for removed loop as it run in separate analysis
                ConstScopeInfo si(state, block, loopStack, visitedLoops);
                blockSuccs.push_back({succBlock, si});

            } else    
            if ( isa<GotoStmt>(term) ) {
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
                
                if (DebugOptions::isEnabled(DebugComponent::doConstBlock)) {
                    cout << "Succ Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                }
                SCT_TOOL_ASSERT (succBlock.isReachable(), "No reachable successor");

                ConstScopeInfo si(state, block, loopStack, visitedLoops);
                blockSuccs.push_back({succBlock, si});
                
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
        for (const auto& bs : blockSuccs) {
            bool found = false;
            auto i = delayed.rbegin();
            unsigned blockID = bs.first.getCfgBlockID();
            
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
            // Insert new scope according with CFG block ID, higher ID goes first
            if (!found) {
                auto j = delayed.begin();
                for (; j != delayed.end(); ++j) {
                    if (blockID < j->first.getCfgBlockID()) {
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
            //cout << "Get next block B" << i->first.getCfgBlockID() << endl;
            
            // Prepare next block
            prepareNextBlock(i->first, i->second);
            
            // Remove block from @delayed, use ++ as @base() points to next object 
            delayed.erase((++i).base());

        } else {
            // Exit from function in non-empty loop stack, means return from loop
            // Do not consider return to main loop in CTHREAD
            if (isCombProcess || !contextStack.empty()) {
                if (!loopStack.empty()) {
                    ScDiag::reportScDiag(funcDecl->getBeginLoc(), 
                                         ScDiag::SYNTH_RETURN_FROM_LOOP);
                }
            }
                
            // No more blocks to analysis 
            if (contextStack.empty()) {
                //state->print();
                break;
            }
            
            // Mark function as not eligible for evaluation as constant if
            // it does not have simple return
            if (sideEffectFunc || !simpleReturnFunc) {
                auto callStack = contextStack.getStmtStack();
                // Skip empty stack as there is no function
                if (!callStack.empty()) {
                    constEvalFuncs[callStack] = NO_VALUE;
                }
                //cout << "Non simple return " << hex << (size_t)callStack.back() << dec << endl;
            }
            
            // Restore callee function context
            restoreContext();
            skipOneElement = true;
            //cout << "Restore context next block B" << block.getCfgBlockID() << endl;
        }
    }
}

// Run for function declaration, the same as @setFunction() and @run()
void ScTraverseConst::run(sc_elab::VerilogModule* verMod,
                          const clang::FunctionDecl* fdecl)
{
    this->verMod = verMod;
    this->funcDecl = fdecl;

    run();
}

// Current thread has reset signal
void ScTraverseConst::setHasReset(bool hasReset_)
{
    hasReset = hasReset_;
}

}