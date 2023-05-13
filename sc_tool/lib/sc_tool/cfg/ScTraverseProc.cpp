/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "sc_tool/cfg/ScTraverseProc.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
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
        //cout << "storeStmtStr " << hex << stmt << dec << " " << *str << endl;
    }
}

// Store statement string for @nullptr
void ScTraverseProc::storeStmtStrNull(Stmt* stmt) 
{
    if (auto str = codeWriter->getStmtString(stmt)) {
        scopeGraph->storeStmt(nullptr, str.getValue());
    }
}

std::shared_ptr<CodeScope> ScTraverseProc::deadScope = make_shared<CodeScope>();


// Create new code scope with specified level
shared_ptr<CodeScope> ScTraverseProc::createScope(unsigned level) 
{
    return make_shared<CodeScope>(level, inMainLoop);
}

// Get code scope for block or create new one
shared_ptr<CodeScope> ScTraverseProc::getScopeForBlock(AdjBlock block, unsigned level)
{
    auto i = delayed.rbegin();
    for (; i != delayed.rend(); ++i) {
        if (i->first == block) {
            SCT_TOOL_ASSERT (!i->second.empty(), "No scope for block");
            return (i->second.front().currScope);
        }
    }
    return createScope(level);
}

// Generate temporal assertion inside of loop(s) if required
llvm::Optional<std::string> 
ScTraverseProc::getSvaInLoopStr(const std::string& svaStr, bool isResetSection) 
{
    std::string tabStr = "        ";
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
    
    loopStr += tabStr + (assertName ? (*assertName+(isResetSection ? "r":"")+
                         " : ") : "") + "assert property ( " + svaStr + " );\n";
    
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

//    cout << "Call stack first iter " << hex;
//    for (auto i : callStack) cout << " " << i;
//    cout << " fval " << fval << endl;
    
    if (firstIter && otherIters) {
        // Join first iteration value to all other iterations value
        if (val != fval) val = NO_VALUE;
    } else 
    if (firstIter && !otherIters) { 
        // If there is only one loop iteration
        val = fval;
    }
    
    //cout << "getTermConds " << hex << stmt << " stackSize " << callStack.size()  
    //     << " val " << val << dec << endl;
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
    for (const auto& si : scopeInfos) {
        // Check the next scope is the same
        SCT_TOOL_ASSERT (nextScope == si.currScope || nextScope == nullptr,
                         "Incorrect next scope");
        nextScope = si.currScope;
    }

    // Update predecessor scopes and return loop stack for the next block
    loopStack = scopeInfos.front().loopStack;
    for (const auto& si : scopeInfos) {
        // This error happens for return in loop, use not fatal error to 
        // report it correctly in TraverseProc
        if (si.loopStack != loopStack) {
            SCT_INTERNAL_ERROR_NOLOC ("Different loop stacks in predecessors");
        }
    }
    
    // Set previous and new current block
    block = nextBlock;
    scopeGraph->setCurrScope(nextScope);
    
    // Join dead code flags by AND form all inputs 
    deadCond = true;
    for (auto&& si : scopeInfos) {
        deadCond = deadCond && si.deadCond;
    }
    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
        cout << "  dead cond " << deadCond << endl;
    }

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
    SCT_TOOL_ASSERT (!inFuncParams, 
                     "Function call or constructor in another call parameter");

    auto i = calledFuncs.emplace(expr, make_pair(retVal, NO_VALUE));
    if (!i.second) {
        cout << hex << expr << dec << endl;
        SCT_TOOL_ASSERT (false, "Second meet of function call");
    }
    //cout << "prepareCallContext expr " << hex << expr << dec << " retVal " << retVal << endl;

    // Prepare current context to store
    lastContext = std::make_shared<ScFuncContext>(
                                CfgCursor(funcDecl, nullptr, 0), 
                                returnValue, modval, recval, delayed, loopStack, 
                                calledFuncs, false, noExitFunc, scopeGraph, 
                                codeWriter->serialize());

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
    if (!funcDecl) {
        ScDiag::reportScDiag(expr->getBeginLoc(),
                             ScDiag::SYNTH_INCORRECT_FUNC_CALL) << "---";
    } 
    
    string fname = funcDecl->getNameAsString();
    QualType retType = funcDecl->getReturnType();
    auto nsname = getNamespaceAsStr(funcDecl);

    if (fname == "__assert" || fname == "__assert_fail") {
        // Do nothing
        
    } else
    if (nsname && *nsname == "sc_core") {
        if (fname == "wait") {
            // SC wait call, wait as function presents in sc_wait.h
            if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                cout << "----------- wait "<< expr->getSourceRange().getBegin().printToString(sm) <<"-----------" << endl;
            }
            waitCall = true;
            
        } else {
            // Do nothing
        }
    } else
    if (nsname && *nsname == "sc_dt") {
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
        
        auto callStack = contextStack.getStmtStack();
        callStack.push_back(expr);
        auto i = constEvalFuncs.find(callStack);
        
        if (i != constEvalFuncs.end()) {
            // Function call evaluated as constant
            SValue rval = i->second;
            codeWriter->putLiteral(expr, rval);
            constReplacedFunc.emplace(expr, expr);
            
            if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                cout << "Replace call with evaluated constant " << rval << endl;
            }
            
        } else {
            // Normal processing of function call 
            if (codeWriter->isParseSvaArg()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_FUNC_IN_ASSERT);
            }
            
            // Declare temporal variable if it is not a pointer
            if (!isVoidType(retType) && !isPointer(retType)) {
                codeWriter->putVarDecl(nullptr, retVal, retType, nullptr, false, 
                                       level);
            }
            
            // Generate function parameter assignments
            prepareCallParams(expr, modval, funcDecl);
            // Register return value and prepare @lastContext
            prepareCallContext(expr, modval, NO_VALUE, funcDecl, retVal);
            // Return value variable has call point level
            state->setValueLevel(retVal, level);
        }
    }
}

// Member function call expression
void ScTraverseProc::parseMemberCall(CXXMemberCallExpr* expr, SValue& tval, 
                                     SValue& val) 
{
    // Get @this expression and its type
    Expr* thisExpr = expr->getImplicitObjectArgument();
    QualType thisType = thisExpr->getType();
    bool isZeroWidth = isZeroWidthType(thisType);
    
    // Parse this expression inside and put result into @tval
    ScGenerateExpr::parseMemberCall(expr, tval, val);
    // Return value passed in @val
    SValue retVal = val;

    // Get method
    FunctionDecl* methodDecl = expr->getMethodDecl()->getAsFunction();
    string fname = methodDecl->getNameAsString();
    QualType retType = methodDecl->getReturnType();
    
    if ( isAnyScIntegerRef(thisType, true) || isZeroWidth ) {
        // Do nothing 
        
    } else 
    if ( isScChannel(thisType) ) {
        // Do nothing 
       
    } else 
    if (isConstCharPtr(expr->getType())) {
        // Do nothing, all logic implemented in ScGenerateExpr
        
    } else
    if ( isAnyScCoreObject(thisType) ) {
        if (fname == "wait") {
            // SC wait call
            if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                cout << "----------- wait "<< expr->getSourceRange().getBegin().printToString(sm) <<"-----------" << endl;
            }
            waitCall = true;
            
        } else {
            // Do nothing for other @sc_core methods
        }
    } else 
    if (codeWriter->isParseSvaArg()) {
        // Do nothing for function call in SVA, all done in ScGenerateExpr
        
    } else {
        // General method call
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for METHOD : " << fname << " (" 
                 << expr->getSourceRange().getBegin().printToString(sm) << ") |" << endl;
            cout << "-------------------------------------" << endl;
        }
                    
        auto callStack = contextStack.getStmtStack();
        callStack.push_back(expr);
        auto i = constEvalFuncs.find(callStack);
        
        if (i != constEvalFuncs.end()) {
            // Function call evaluated as constant
            SValue rval = i->second;
            codeWriter->putLiteral(expr, rval);
            constReplacedFunc.emplace(expr, expr);
            
            if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                cout << "Replace call with evaluated constant " << rval << endl;
            }
            
        } else {
            // Declare temporal variable if it is not a pointer
            if (!isVoidType(retType) && !isPointer(retType)) {
                codeWriter->putVarDecl(nullptr, retVal, retType, nullptr, false, 
                                       level);
            }

            // Get record from variable/dynamic object, no unknown index here
            SValue ttval = getRecordFromState(tval, ArrayUnkwnMode::amNoValue);
            //cout << "parseMemberCall tval " << tval << ", ttval " << ttval << endl;

            // This value *this must be a kind of record
            if (!ttval.isRecord()) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::SYNTH_INCORRECT_RECORD) 
                                << tval.asString() << ttval.asString();
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
            if (methodDecl->isVirtualAsWritten() && !hasClassCast) {
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

            if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                cout << "Function call this class value " << ttval.asString()
                     << ", dynamic class value " << dyntval.asString() 
                     << ", funcModval " << funcModval.asString() << endl;
            }
            
            // Register SS channels which needs to be added into sensitivity
            if ( isSctChannelSens(funcModval.getType(), methodDecl) ) {
                //cout << "TraverseProc funcModval " << funcModval << " ttval " << ttval << endl;
                // Skip @sct_target if it has FIFO inside
                if (isSctTarg(modval.getType()) && isSctFifo(funcModval.getType())) {
                    auto elabObj = state->getElabObject(modval);
                    skipSctTargets.insert(*elabObj);
                }
                
                if (auto elabObj = state->getElabObject(funcModval)) {
                    usedSctChannels.insert(*elabObj);
                }
            }

            // Generate function parameter assignments
            prepareCallParams(expr, funcModval, methodDecl);
            // Register return value and prepare @lastContext
            prepareCallContext(expr, funcModval, NO_VALUE, methodDecl, retVal);        
            // Return value variable has call point level
            state->setValueLevel(retVal, level);
            
            // Set record expression (record name and indices) to use in called function
            //cout << "\nthisExpr " << hex << thisExpr << dec << endl;
            if (auto thisStr = codeWriter->getStmtString(thisExpr)) {
                //std::cout << "setRecordName " << funcModval << " " << thisStr.getValue() << std::endl; 
                codeWriter->setRecordName(funcModval, thisStr.getValue());
            }
        }
    }
}

// Operator call expression
void ScTraverseProc::parseOperatorCall(CXXOperatorCallExpr* expr, SValue& tval,
                                       SValue& val) 
{
    SCT_TOOL_ASSERT (expr->getNumArgs() != 0, "Operator without arguments");
    Expr* thisExpr = expr->getArgs()[0];
    bool isZeroWidth = isZeroWidthType(thisExpr->getType());
    
    // Parse this expression inside and put result into @tval
    ScGenerateExpr::parseOperatorCall(expr, tval, val);

    // Get operator method
    FunctionDecl* methodDecl = expr->getCalleeDecl()->getAsFunction();
    string fname = methodDecl->getNameAsString();
    QualType retType = methodDecl->getReturnType();
    
    OverloadedOperatorKind opcode = expr->getOperator();
    bool isAssignOperator = expr->isAssignmentOp() && opcode == OO_Equal;
    bool isSctChan = isAssignOperatorSupported(tval.getType());
    
    if (isZeroWidth) {
        // Do nothing 
        
    } else 
    if (isAssignOperator && isSctChan) {
        // Operator call in sct namespace
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "-------------------------------------" << endl;
            cout << "| Build CFG for OPERATOR : " << fname << " (" 
                 << expr->getSourceRange().getBegin().printToString(sm) << ") |\n";
            cout << "-------------------------------------" << endl;
        }
        
        // Return value passed in @val
        SValue retVal = val;
        
        // Declare temporal variable if it is not a pointer
        if (!isVoidType(retType) && !isPointer(retType)) {
            codeWriter->putVarDecl(nullptr, retVal, retType, nullptr, false, level);
        }

        // Get record from variable/dynamic object, no unknown index here
        SValue ttval = getRecordFromState(tval, ArrayUnkwnMode::amNoValue);
        //cout << "parseMemberCall tval " << tval << ", ttval " << ttval << endl;

        // This value *this must be a kind of record
        if (!ttval.isRecord()) {
            ScDiag::reportScDiag(expr->getBeginLoc(),
                                 ScDiag::SYNTH_INCORRECT_RECORD) 
                            << tval.asString() << ttval.asString();
        }

        // Dynamic class for member record
        SValue dyntval = ttval;
        // @modval for called function analysis
        SValue funcModval = ttval;

        // Virtual operators not supported
        if (methodDecl->isVirtualAsWritten()) {
            SCT_INTERNAL_ERROR(expr->getBeginLoc(), "No virtual operator supported");
        }

        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "Function call this class value " << ttval.asString()
                 << ", dynamic class value " << dyntval.asString() 
                 << ", funcModval " << funcModval.asString() << endl;
        }

        // Generate function parameter assignments
        prepareCallParams(expr, funcModval, methodDecl);
        // Register return value and prepare @lastContext
        prepareCallContext(expr, funcModval, NO_VALUE, methodDecl, retVal);        
        // Return value variable has call point level
        state->setValueLevel(retVal, level);

        // Get @this expression 
        Expr** args = expr->getArgs();
        Expr* thisExpr = args[0];

        // Set record expression (record name and indices) to use in called function
        //cout << "\nthisExpr " << hex << thisExpr << dec << endl;
        if (auto thisStr = codeWriter->getStmtString(thisExpr)) {
            //std::cout << "setRecordName " << funcModval << " " << thisStr.getValue() << std::endl; 
            codeWriter->setRecordName(funcModval, thisStr.getValue());
        }
    }
}

// Choose and run DFS step in accordance with expression type.
void ScTraverseProc::chooseExprMethod(Stmt *stmt, SValue &val)
{
    //cout << "--- chooseExprMethod " << hex << stmt << dec << endl;    
    // Constructor call for local record considered as function call
    bool anyFuncCall = isa<CallExpr>(stmt) || isa<CXXConstructExpr>(stmt);

    if (anyFuncCall && calledFuncs.count(stmt)) {
        const auto& returns = calledFuncs.at(stmt);
        val = returns.first;
        //cout << "Get RET value " << val << " for stmt #" << hex << stmt << dec  << endl;

        // Store return value as term for call statement
        if (returns.second) {
            // For function returns pointer use pointed object instead of 
            // temporary variable
            codeWriter->putValueExpr(stmt, returns.second);
        } else 
        if (val) {
            codeWriter->putValueExpr(stmt, val);
        }

    } else {
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
    SCT_TOOL_ASSERT (constReplacedFunc.empty(), "@constReplacedFunc is not empty");
    SCT_TOOL_ASSERT (funcDecl, "Function declaration and context stack not set");

    // Clear temporary variable index generator
    STmpVariable::clearId();
    
    scopeGraph = std::make_shared<ScScopeGraph>();
    auto firstScope = createScope(0);
    scopeGraph->setFirstScope(firstScope);
    scopeGraph->setCurrScope(firstScope);
    scopeGraph->setName(funcDecl->getNameAsString());
    scopeGraph->setCurrLevel(0);
    level = 0;
    
    // Setup first non-MIF module value
    synmodval = state->getSynthModuleValue(modval, ScState::MIF_CROSS_NUM);
    
    // Check if current module if element of array of MIF
    if (isScModularInterface(modval.getType())) {
        // Get all MIF arrays up to the parent module
        auto mifarrs = state->getAllMifArrays(modval, ScState::MIF_CROSS_NUM);
        
        string s;
        for (const SValue& val : mifarrs) {
            SCT_TOOL_ASSERT (val.isArray() && !val.getArray().isUnknown(), 
                             "Unknown index for MIF array element");
            auto i = val.getArray().getOffset();
            s += "["+ to_string(i) +"]";
        }
        
        if (!mifarrs.empty()) {
            //std::cout << "\nsetMIFName " << modval << " " << s << std::endl;
            codeWriter->setMIFName(modval, s);
        }
    }

    codeWriter->setRecordName(NO_VALUE, "");

    cfg = cfgFabric->get(funcDecl);
    block = AdjBlock(&cfg->getEntry(), true);
    elemIndx = 0;
    exitBlockId = cfg->getExit().getBlockID();
    
    condFuncCallLoops.clear();
    
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

    scopeGraph = context.scopeGraph;
    level = scopeGraph->getCurrLevel();
    
    codeWriter->deserialize(context.codeWriter);

    if (funcCallRestore) {
        // Restore after function call and break/continue
        calledFuncs = context.calledFuncs;
        delayed = context.delayed;
        noExitFunc = context.noExitFunc;
        
        // Store pointed object returned from function if required
        if (returnPtrVal) {
            for (auto& entry : calledFuncs) {
                if (entry.second.first == returnValue) {
                    entry.second.second = returnPtrVal;
                    break;
                }
            }
        }
        returnPtrVal = NO_VALUE;
        
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
        noExitFunc = true;
        
        // All loops becomes removed to avoid break/continue generating
        for (auto& i : loopStack) {
            i.removed = true;
        }
    }

    returnValue = context.returnValue;
    funcDecl = context.callPoint.getFuncDecl();

    cfg = cfgFabric->get(funcDecl);
    block = AdjBlock(const_cast<CFGBlock*>(context.callPoint.getBlock()), true);
    // Start analysis with restored element index
    elemIndx = context.callPoint.getElementID();
    exitBlockId = cfg->getExit().getBlockID();
    
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << endl;
        cout << "---------- restoreContext ------------ " << funcCallRestore << endl;
        cout << "Func " << funcDecl->getNameAsString() 
             <<  ", modval " << modval << ", level " << level << endl;
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
    // Skip block in context restore after break/continue in removed loop and
    // for function call with @wait() at all paths
    bool skipOneBlock = false;
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

        // Fill statement levels for entry function
        stmtInfo.run(funcDecl, 0);
    
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
    
    SCT_TOOL_ASSERT (isCombProcess || findWaitInLoop, 
                     "No wait() finder for CTHREAD process");

    while (true)
    {
        Stmt* currStmt = nullptr;
        
        unsigned isNotLibrarySpace = 1;
        if (funcDecl) {
            auto nsname = getNamespaceAsStr(funcDecl);
            if (nsname && (*nsname == "std" || *nsname == "sc_core" || 
                           *nsname == "sct")) {
                isNotLibrarySpace = 0;
            }
        }
        
        // Unreachable block can be not dead code after &&/|| condition        
        // Do not generate statement for dead code block
        if (!deadCond && !skipOneBlock) {
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

                    if (auto doLevel = stmtInfo.getLevel(const_cast<DoStmt*>(doterm))) {
                        level = *doLevel;
                        //cout << "Term (1) #" << doterm << " level " << level << endl;
                    } else {
                        cout << "DO term #" << doterm << " no level "<< endl;
                        doterm->dumpColor();
                        SCT_INTERNAL_ERROR(currStmt->getBeginLoc(), 
                                           "No level found for terminator");
                    }
                    scopeGraph->setCurrLevel(level);
                    
                    // Create separate body scope to set @level+1 for it
                    auto bodyScope = createScope(scopeGraph->getCurrLevel()+1);
                    scopeGraph->addScopeSucc(bodyScope);
                    scopeGraph->addScopePred(bodyScope, scopeGraph->getCurrScope());
                    scopeGraph->setCurrScope(bodyScope);
                }

                // Add current loop into loop stack
                loopStack.pushLoop({doterm, scopeGraph->getCurrLevel(), ifStmt});
                //cout << "Loop level "<< scopeGraph->getCurrLevel() << endl;
                if (isNotLibrarySpace) statTerms.insert(doterm);
            }

            // CFG block body analysis, preformed for not dead state only
            for (size_t i = elemIndx; i < block.getCfgBlock()->size(); ++i)
            {
                const CFGElement& elm = block.getCfgBlock()->operator [](i);
                if (elm.getKind() == CFGElement::Kind::Statement) {
                    // Get statement 
                    CFGStmt cfgstmt = elm.getAs<CFGStmt>().getValue();
                    currStmt = const_cast<Stmt*>(cfgstmt.getStmt());
                    
                    // Skip bind temporary required for function return 
                    // which is not assigned to variable
                    if (auto bindstmt = dyn_cast<CXXBindTemporaryExpr>(currStmt)) {
                        auto bindexpr = bindstmt->getSubExpr();
                        if (isUserCallExpr(bindexpr)) continue;
                    }
                    
                    // Get statement level and check if it is sub-statement
                    bool isStmt = false; 
                    bool isCallSubStmt = false;
                    bool isZeroWidth = isZeroWidthCall(currStmt);
                    if (auto stmtLevel = stmtInfo.getLevel(currStmt)) {
                        level = *stmtLevel;
                        isStmt = !isZeroWidth;

                    } else 
                    if (auto stmtLevel = stmtInfo.getDeclGroupLevel(currStmt)) {
                        level = *stmtLevel;
                        isStmt = !isZeroWidth;
                        
                    } else 
                    if (auto superStmt = stmtInfo.getSuperStmt(currStmt)) {
                        if (auto superStmtLevel = stmtInfo.getLevel(superStmt)) {
                            level = *superStmtLevel;
                            isCallSubStmt = isUserCallExpr(currStmt);
                            isStmt = isCallSubStmt && !isZeroWidth &&
                                     !isIoStreamStmt(superStmt) &&
                                     !codeWriter->isParseSvaArg();
                        }
                        //cout << hex << "getSubStmtLevel " << currStmt << endl;
                        
                    } else {
                        cout << hex << "#" << currStmt << dec << endl;
                        currStmt->dumpColor();
                        SCT_INTERNAL_ERROR(currStmt->getBeginLoc(), 
                                           "No level found for sub-statement");
                    }
                    
                    // Check function call is not evaluated as constant
                    if (isStmt) {
                        auto callStack = contextStack.getStmtStack();
                        callStack.push_back(currStmt);
                        isStmt = constEvalFuncs.count(callStack) == 0;
                        //cout << "isCallSubStmt " << isStmt << " " << hex << currStmt << dec << endl;
                    }

                    // Set level for statement and sub-statement
                    scopeGraph->setCurrLevel(level);
                    // Skip sub-statement
                    if (!isStmt) continue;
                    
                    if (DebugOptions::isEnabled(DebugComponent::doGenStmt)) {
                        cout << endl;
                        currStmt->dumpColor();
                        //state->print();
                        cout << " level " << level << endl;
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
                    
                    // Skip non-live statements which determined in CPA
                    if (liveStmts.find(currStmt) == liveStmts.end()) continue;

                    // Parse statement expression and generate RTL
                    auto stmtStr = parse(currStmt);
                    SCT_TOOL_ASSERT (!assignLHS, "Incorrect assign LHS flag");
                    
                    // Store statement if it has term only, that ignores 
                    // declaration statements, also check for empty statement
                    // for example function call w/o result assignment
                    if (stmtStr) {
                        if (isTemporalAssert(currStmt)) {
                            // Temporal assertion in process
                            if (!inMainLoop) {
                                if (!noSvaGenerate) {
                                    if (isResetSection) {
                                        if (auto str = getSvaInLoopStr(*stmtStr, 1)) {
                                            sctRstAsserts.insert(*str);
                                        }
                                    }
                                    if (auto str = getSvaInLoopStr(*stmtStr, 0)) {
                                        sctAsserts.insert(*str);
                                    }
                                }
                            } else {
                                ScDiag::reportScDiag(currStmt->getBeginLoc(),
                                                ScDiag::SYNTH_SVA_IN_MAIN_LOOP);
                            }
                            if (isNotLibrarySpace) statAsrts.insert(currStmt);

                        } else 
                        if (isSctAssert(currStmt)) {
                            // Immediate assertion in process, @sct_assert
                            scopeGraph->storeAssertStmt(currStmt, *stmtStr);
                            if (emptySensitivity) {
                                scopeGraph->setEmptySensStmt(currStmt);
                            }
                            if (isNotLibrarySpace) statAsrts.insert(currStmt);
                        
                        } else {
                            // Normal statement or sub-statement with call
                            Stmt* stmt = isCallSubStmt ?
                                         stmtInfo.getSuperStmt(currStmt) : nullptr;
                            if (stmt && (isa<ForStmt>(stmt) ||
                                isa<WhileStmt>(stmt) || isa<DoStmt>(stmt))) {
                                // Function call in loop condition
                                if (isCombProcess) {
                                    ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                            ScDiag::SYNTH_FUNC_CALL_COND_LOOP);
                                } else {
                                    // Such loop must have @wait()
                                    if (!findWaitInLoop->hasWaitCall(stmt)) {
                                        ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                                ScDiag::SYNTH_FUNC_CALL_COND_LOOP);
                                    }
                                    // Loop condition cannot have @wait()
                                    if (findWaitInLoop->hasWaitCall(currStmt)) {
                                        ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                                ScDiag::SYNTH_FUNC_CALL_WAIT_LOOP);
                                    }
                                }

                                // Function call stored in scope graph right 
                                // before the loop statement
                                auto i = condFuncCallLoops.emplace(
                                     stmt, vector<pair<Stmt*, std::string>>());
                                auto& condFuncCalls = i.first->second;
                                
                                bool found = false;
                                for (auto entry : condFuncCalls) {
                                    if (entry.first == currStmt) found = true;
                                }
                                if (!found) {
                                    condFuncCalls.emplace_back(currStmt, *stmtStr);
                                }
                            } else {
                                // Normal statement, store it in scope graph
                                scopeGraph->storeStmt(currStmt, *stmtStr);  
                                
                                // Add comment for constant evaluated function
                                auto i = constReplacedFunc.find(currStmt);
                                if (i != constReplacedFunc.end()) {                   
                                    auto callExpr = dyn_cast<CallExpr>(i->second);
                                    auto funcDecl = callExpr->getDirectCallee();
                                    if (!funcDecl) {
                                        ScDiag::reportScDiag(callExpr->getBeginLoc(),
                                            ScDiag::SYNTH_INCORRECT_FUNC_CALL) << "---";
                                    }

                                    string s = "Call of "+funcDecl->getNameAsString()+"()";
                                    scopeGraph->addComment(currStmt, s);
                                }
                            }
                            if (emptySensitivity) {
                                scopeGraph->setEmptySensStmt(currStmt);
                            }
                            if (isNotLibrarySpace) statStmts.insert(currStmt);
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
                                calledFuncs, false, true, scopeGraph,
                                codeWriter->serialize());
                        
                        //cout << " waitCall, level " << scopeGraph->getCurrLevel() << endl;
                        
                        // Add current call statement to the context, use clone
                        // to allocate scope graph pointer to avoid changing it
                        contextStack.push_back(waitCntx);
                        auto waitCntxStack = contextStack.clone(inMainLoop);
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
                        
                        if (!isSingleStateThread) {
                            // @false no tabulation added
                            scopeGraph->storeStateAssign(
                                    currStmt, state->getProcStateName(),
                                    waitId, isResetSection, false,
                                    getFileName(currStmt->getSourceRange().
                                    getBegin().printToString(sm)));
                        }
                        
                        if (isNotLibrarySpace) statWaits.insert(currStmt);
                        
                        // Skip analysis rest of the block after wait()
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
                            SCT_INTERNAL_FATAL(currStmt->getBeginLoc(), 
                                "No function body found, probably STL function");
                        }
                        block = AdjBlock(&cfg->getEntry(), true);
                        exitBlockId = cfg->getExit().getBlockID();
                        
                        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
                            cout << "--------------------------------------" << endl;
                            cfg->dump(LangOptions(), true);
                            cout << "--------------------------------------" << endl;
                        }
                        
                        // Create new scope graph
                        scopeGraph = std::make_shared<ScScopeGraph>();
                        auto firstScope = createScope(level+1);
                        scopeGraph->setFirstScope(firstScope);
                        scopeGraph->setCurrScope(firstScope);
                        // Set function name
                        scopeGraph->setName(funcDecl->getNameAsString());

                        // Fill levels for current function, use level+1 to 
                        // distinguish function scope from current one
                        stmtInfo.run(funcDecl, level+1);  

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
                        noExitFunc = true;
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
        
        // Block successors
        vector<pair<AdjBlock, ScopeInfo> >  blockSuccs;
        // Block terminator
        auto cfgBlock = block.getCfgBlock();
        SCT_TOOL_ASSERT(cfgBlock, "Current CFG block is null");

        Stmt* term = cfgBlock->getTerminator().getStmt();
        if (term && liveTerms.find(term) == liveTerms.end()) {
            SCT_INTERNAL_FATAL (term->getBeginLoc(), "Dead terminator in code");
        }
        
        bool breakInRemovedLoop = false;
        bool contInRemovedLoop = false;
        
        if (skipOneBlock) {
            // Skip block terminator in context restore after break/continue 
            // in removed loop and function call with wait() at all paths

        } else 
        if (waitCall) {
            // Skip rest of block after @wait() and terminator
            waitCall = false;
            
        } else
        if (funcCall) {
            // Suspend analysis of this function and go to to called function
            funcCall = false;
            continue;
            
        } else
        if (term) {
            // Set level for loop terminator
            if (auto termLevel = stmtInfo.getLevel(term)) {
                level = *termLevel;
                //cout << "Term (1) #" << term << " level " << level << endl;
            } else 
            if (auto termLevel = stmtInfo.getSubStmtLevel(term)) {
                level = *termLevel;
                //cout << "Term (2) #" << term << " level " << level << endl;
            } else {
                cout << "Term #" << term << " no level "<< endl;
                term->dumpColor();
                SCT_INTERNAL_ERROR(currStmt->getBeginLoc(), 
                                   "No level found for terminator");
            }
            scopeGraph->setCurrLevel(level);

            // Get terminator condition value for all/first iteration from CPA
            SValue termCValue; SValue termCValueFirst;
            getTermCondValue(term, termCValue, termCValueFirst);
            
            bool trueCond = termCValue.isInteger() && 
                            !termCValue.getInteger().isNullValue();
            bool falseCond = termCValue.isInteger() && 
                             termCValue.getInteger().isNullValue();
            
            if (DebugOptions::isEnabled(DebugComponent::doGenTerm)) {
                term->dumpColor();
                cout << "Terminator: level " << scopeGraph->getCurrLevel() 
                     << ", termCValue " << termCValue.asString() << endl;
            }
            
            if (IfStmt* ifstmt = dyn_cast<IfStmt>(term)) {
                // Two successor blocks
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2, 
                                 "No two successors in IF");
                // Then block always exists even if it is empty
                // If Else branch is empty, Else block is next IF block
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock thenBlock(*iter);
                AdjBlock elseBlock(*(++iter));
                SCT_TOOL_ASSERT (thenBlock != elseBlock,
                                 "The same then and else blocks in IF");
                
                if (!falseCond) {
                    auto thenScope = getScopeForBlock(thenBlock, level+1);
                    scopeGraph->addScopeSucc(thenScope);
                    
                    ScopeInfo thenES(scopeGraph->getCurrScope(), thenScope, 
                                     loopStack);
                    blockSuccs.push_back({thenBlock, thenES});

                } else {
                    scopeGraph->addScopeSucc(deadScope);
                }

                if (!trueCond) {
                    auto elseScope = getScopeForBlock(elseBlock, level+1);
                    // For empty Else create empty scope and connect it
                    if ( checkIfBranchEmpty(ifstmt->getElse()) ) {
                        auto emptyScope = createScope(level+1);
                        scopeGraph->addScopeSucc(emptyScope);
                        scopeGraph->addScopePred(emptyScope, scopeGraph->getCurrScope());
                        scopeGraph->addScopeSucc(emptyScope, elseScope);
                    } else {
                        scopeGraph->addScopeSucc(elseScope);
                    }

                    ScopeInfo elseES(scopeGraph->getCurrScope(), elseScope,
                                     loopStack);
                    blockSuccs.push_back({elseBlock, elseES});
                    
                } else {
                    scopeGraph->addScopeSucc(deadScope);
                }
                
                // Parse and store IF statement
                auto stmtStr = parseTerm(term, termCValue);
                if (!emptySensitivity) {
                    scopeGraph->storeStmt(term, stmtStr.getValue());
                }
                if (isNotLibrarySpace) statTerms.insert(term);

            } else
            if (SwitchStmt* swstmt = dyn_cast<SwitchStmt>(term))
            {
                // Switch cases, skip first case as it belongs to @default
                const SwitchCase* swcase = swstmt->getSwitchCaseList();
                bool hasDefault = false;
                SwitchCase* defcase = nullptr;

                if (swcase && isa<const DefaultStmt>(swcase) ) {
                    hasDefault = true;
                    defcase = const_cast<SwitchCase*>(swcase);
                    swcase = swcase->getNextSwitchCase();
                }
                
                // Cases with their blocks, reordered in direct order
                auto cases = getSwitchCaseBlocks(swcase, cfgBlock);
                bool constCase = false; // One case chosen by constant condition
                
                for (auto entry : cases) {
                    SwitchCase* swcase = const_cast<SwitchCase*>(entry.first);
                    AdjBlock caseBlock = entry.second;
                    SCT_TOOL_ASSERT (caseBlock.getCfgBlock(), "No case block");
                    bool emptyCase = isCaseEmpty(caseBlock);
                   
                    // Get case expression and store it in generator
                    auto cstmt = dyn_cast<CaseStmt>(swcase);
                    SCT_TOOL_ASSERT (cstmt, "Unexpected statement type in switch");
                    Expr* caseExpr = cstmt->getLHS();
                    // Evaluate case value
                    SValue caseValue = evaluateConstInt(caseExpr).second;

                    if (termCValue.isInteger() && caseValue.isInteger()) {
                        bool A = APSInt::isSameValue(termCValue.getInteger(), 
                                                     caseValue.getInteger());
                        constCase = constCase || A;
                    }
                    
                    // As @liveTerms is not context sensitive it is possible 
                    // some dead cases not eliminated
                    bool deadCase = liveTerms.find(swcase) == liveTerms.end();
                    
                    if (!deadCase) {
                        // Create new/reuse scopes for case branch
                        auto caseScope = getScopeForBlock(caseBlock, level+1);
                        scopeGraph->addScopeSucc(caseScope);

                        // Add predecessor for empty case scope as it is not analyzed
                        if (emptyCase) {    
                            scopeGraph->addScopePred(caseScope, 
                                                     scopeGraph->getCurrScope());
                        } else {
                            // Do not analyze empty cases as that leads to mix blocks ID,
                            // in scopeGraph its string is taken from next non-empty case
                            ScopeInfo caseSI(scopeGraph->getCurrScope(), caseScope,
                                             loopStack);
                            blockSuccs.push_back({caseBlock, caseSI});
                        }
                    
                        // Store case expression value
                        auto caseStr = parse(caseExpr);
                        SCT_TOOL_ASSERT(caseStr, "No case statement string");
                        scopeGraph->storeCase(caseScope, *caseStr, emptyCase);

                    } else {
                        scopeGraph->addScopeSucc(deadScope);
                    }
                }

                // Last successor block is default or after switch block
                AdjBlock succBlock(*cfgBlock->succ_rbegin());
                
                // Set loop exit to avoid level up in taking @switch next block
                bool deadDefault = hasDefault && 
                                   liveTerms.find(defcase) == liveTerms.end();
                if (!hasDefault || !deadDefault) {
                    auto succScope = getScopeForBlock(succBlock, hasDefault ? 
                                                      level+1 : level);
                    scopeGraph->addScopeSucc(succScope);

                    ScopeInfo switchSI(scopeGraph->getCurrScope(), succScope,
                                       loopStack);
                    blockSuccs.push_back({succBlock, switchSI});
                    
                    if (hasDefault) {
                        scopeGraph->storeCase(succScope, "default", false);
                    }
                } else {
                    scopeGraph->addScopeSucc(deadScope);
                }
                
                // Parse and store switch statement
                auto stmtStr = parseTerm(term, termCValue);
                // Write switch code only if there are several branches
                if (cfgBlock->succ_size() > 1) {
                    scopeGraph->storeStmt(term, stmtStr.getValue());
                }
                if (isNotLibrarySpace) statTerms.insert(term);

            } else 
            if (BinaryOperator* binstmt = dyn_cast<BinaryOperator>(term))
            {
                BinaryOperatorKind opcode = binstmt->getOpcode();
                SCT_TOOL_ASSERT (opcode == BO_LOr || opcode == BO_LAnd,
                                 "Incorrect terminator statement");

                // For && Then branch goes to right part of && / final IF, 
                // Else branch can go to right part of && or else/exit block
                // For || Then branch can go to right part of && or else/exit block, 
                // Else branch goes to right part of && / final IF
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in ||/&&");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock thenBlock(*iter);
                AdjBlock elseBlock(*(++iter));
                
                // Add only scope towards final IF/Loop/? as body block could
                // be removed if final terminator condition is false
                // @deadThen/Else not consider @deadCond required for complex conditions
                if (opcode == BO_LOr) {
                    auto elseScope = getScopeForBlock(elseBlock, level);
                    scopeGraph->addScopeSucc(elseScope);

                    ScopeInfo elseSI(scopeGraph->getCurrScope(), elseScope,
                                     loopStack, trueCond);
                    blockSuccs.push_back({elseBlock, elseSI});
                }
                
                if (opcode == BO_LAnd) {
                    auto thenScope = getScopeForBlock(thenBlock, level);
                    scopeGraph->addScopeSucc(thenScope);

                    ScopeInfo thenSI(scopeGraph->getCurrScope(), thenScope,
                                     loopStack, falseCond);
                    blockSuccs.push_back({thenBlock, thenSI});
                }

            } else
            if ( isa<ConditionalOperator>(term) )
            {
                // Two argument blocks    
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 2,
                                 "No two successors in cond operator");
                CFGBlock::succ_iterator iter = cfgBlock->succ_begin();
                AdjBlock argBlock1(*iter);
                AdjBlock argBlock2(*(++iter));

                // Arguments have +1 level to return the same level for ? body 
                if (!falseCond) {
                    auto argScope1 = getScopeForBlock(argBlock1, level+1);
                    scopeGraph->addScopeSucc(argScope1);
                    
                    ScopeInfo argSI1(scopeGraph->getCurrScope(), argScope1,
                                     loopStack);
                    blockSuccs.push_back({argBlock1, argSI1});
                    
                }
                if (!trueCond) {
                    auto argScope2 = getScopeForBlock(argBlock2, level+1);
                    scopeGraph->addScopeSucc(argScope2);

                    ScopeInfo argSI2(scopeGraph->getCurrScope(), argScope2,
                                     loopStack);
                    blockSuccs.push_back({argBlock2, argSI2});
                }
                if (isNotLibrarySpace) statTerms.insert(term);
                
            } else                 
            if (isa<ForStmt>(term) || isa<WhileStmt>(term) || isa<DoStmt>(term))
            {
                // Normally one iteration of loop is passed
                auto doWhile = dyn_cast<DoStmt>(term);
                // Determine loop already visited with checking if 
                // current (last) loop has the same terminator
                bool loopVisited = doWhile || loopStack.isCurrLoop(term);
                // DoWhile with false condition
                bool skipDoWhile = doWhile && falseCond;
                
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

                if (findWaitInLoop && findWaitInLoop->hasWaitCall(term)) {
                    // Loop with wait in thread 
                    SCT_TOOL_ASSERT (!isCombProcess, 
                                     "Wait in combinational process");
                    
                    // Loop condition value at current iteration (first or others)
                    SValue iterCValue = (loopVisited) ? 
                                        termCValue : termCValueFirst;
                    
                    noLoopStmt = !loopVisited && iterCValue.isInteger() && 
                                 !iterCValue.getInteger().isNullValue();
                    skipLoop = !loopVisited && iterCValue.isInteger() && 
                                iterCValue.getInteger().isNullValue();
                    // Check @skipDoWhile required for do/while with wait()
                    ifStmt = (loopVisited || iterCValue.isUnknown()) && 
                             !skipDoWhile;

                    // Set enter into main loop, used to reduce level by 1
                    // in ScopeGRaph and to check SVA generation
                    if (!inMainLoop && term == mainLoopStmt) {
                        inMainLoop = true;
                    }
                    
                } else {
                    // Loop without wait in thread or method
                    // One condition for all iterations
                    skipLoop = !loopVisited && falseCond;
                }
                if (DebugOptions::isEnabled(DebugComponent::doGenLoop)) {
                    cout << "LOOP : noLoopStmt " << noLoopStmt << ", skipLoop " << skipLoop 
                         << ", ifStmt " << ifStmt << ", exitBlock " << exitBlock.getCfgBlock() 
                        << ", loopVisited " << loopVisited << endl;
                }
                
                shared_ptr<CodeScope> nextScope = nullptr;
                shared_ptr<CodeScope> elseScope = nullptr;
                bool addEmptyScope = false;

                if (ifStmt) {
                    // Create IF statement instead of loop
                    SCT_TOOL_ASSERT (bodyBlock.getCfgBlock(), 
                                     "Loop body block in NULL");
                    succBlock = bodyBlock;
                    nextScope = getScopeForBlock(succBlock, level+1);

                    // Get CFGBlock* for @exitBlock here
                    if (exitBlock.getCfgBlock()) {
                        // Parse loop as IF statement 
                        auto stmtStr = parseTerm(term, termCValue, true);
                        // Store for-loop initializer and increment
                        if (auto forstmt = dyn_cast<ForStmt>(term)) {
                            bool loopFirstIter = !loopVisited;
                            if (loopFirstIter) {
                                if (Stmt* init = forstmt->getInit()) {
                                    storeStmtStr(init);
                                }
                            } else {
                                if (Expr* inc = forstmt->getInc()) {
                                    storeStmtStr(inc);
                                }
                            }
                        }
                        
                        // Store function call statement in FOR condition
                        // after initialization/increment sections
                        auto i = condFuncCallLoops.find(term);
                        if (i != condFuncCallLoops.end()) {
                            for (const auto& entry : i->second) {
                                scopeGraph->storeStmt(entry.first, entry.second);
                            }
                        }
                        
                        // Store IF statement in code writer
                        scopeGraph->storeStmt(term, stmtStr.getValue(), true);
                        
                        // Get scope for exit block, it cannot be achieved 
                        // from any path through THEN(loop body) block
                        auto exitScope = getScopeForBlock(exitBlock, level);
                        
                        // Prepare else scope empty
                        elseScope = createScope(level+1);
                        scopeGraph->addScopePred(elseScope, scopeGraph->getCurrScope());
                        scopeGraph->addScopeSucc(elseScope, exitScope);

                        // Remove current loop for exit block
                        loopStack.popLoop(term);

                        // Exit scope has the same level, use updated loop stack
                        // and loop exit here to remove this loop at exit
                        ScopeInfo si(scopeGraph->getCurrScope(), exitScope, 
                                     loopStack);
                        // Exit block will be analyzed after body because of level
                        blockSuccs.push_back({exitBlock, si});
                        
                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "Exit block B" << exitBlock.getCfgBlockID() 
                                 << " scope " << exitScope->asString() << endl;
                        }
                    }
                    
                    // Add current loop for loop body 
                    loopStack.pushLoop({term, level, true});
                    
                    if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                        cout << "Body block B" << succBlock.getCfgBlockID() 
                             << " scope " << nextScope->asString() << endl;
                    }

                } else 
                // Body block may be NULL for loop with false condition
                if (!loopVisited && bodyBlock.getCfgBlock() && !skipLoop) {
                    // Enter loop body
                    succBlock = bodyBlock;
                    nextScope = getScopeForBlock(succBlock, level+1);
                    
                    // Add null statement to prevent empty loop scope 
                    // required to have level up and exit from the loop
                    if (!noLoopStmt) {
                        auto emptyScope = createScope(level+1);
                        scopeGraph->addScopeSucc(emptyScope);
                        scopeGraph->addScopePred(emptyScope, scopeGraph->getCurrScope());
                        scopeGraph->addScopeSucc(emptyScope, nextScope);
                        addEmptyScope = true;
                    }

                    if (!noLoopStmt) {
                        // Loop without wait()
                        // Parse loop 
                        auto stmtStr = parseTerm(term, termCValue, false);
                        // Store loop statement in code writer
                        scopeGraph->storeStmt(term, stmtStr.getValue());

                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "Loop Body Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                        }
                        
                    } else {
                        // No loop generated, level not changed
                        // Parse for-loop initialization part only
                        if (auto forstmt = dyn_cast<ForStmt>(term)) {
                            // Parse and store statement string
                            if (auto init = forstmt->getInit()) {
                                SValue val; chooseExprMethod(init, val);
                                storeStmtStr(init);
                            }
                        }
                                      
                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "No loop statement generated Block B"<< succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                        }
                    }
                    
                    // Add current loop into loop stack
                    loopStack.pushLoop({term, scopeGraph->getCurrLevel(), 
                                        noLoopStmt});
                } else 
                // If body block is NULL or loop is skipped, go to exit block
                if (loopVisited || !bodyBlock.getCfgBlock() || skipLoop) {
                    
                    // Parse for-loop initialization part only
                    if (!loopVisited) {
                        if (auto forstmt = dyn_cast<ForStmt>(term)) {
                            if (auto init = forstmt->getInit()) {
                                // Parse and store statement string for external
                                // counter only
                                if (hasForExtrCntr(term)) {
                                    SValue val; chooseExprMethod(init, val);
                                    storeStmtStr(init);
                                }
                            }
                        }
                    }
                    
                    if (exitBlock.getCfgBlock()) {
                        // Exit from the loop, can be already reached by break exit 
                        succBlock = exitBlock;
                        nextScope = getScopeForBlock(succBlock, level);
                        
                        if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                            cout << "Loop Exit Block B" << succBlock.getCfgBlockID() << ((succBlock.isReachable()) ? "" : " is unreachable") << endl;
                        }
                    }

                    // If entered into loop body enter, remove current loop
                    loopStack.popLoop(term);
                }

                // If there is successor block, add it to next analysis
                if (succBlock.getCfgBlock()) {
                    // Set @nextScope as successor of current scope
                    if (!addEmptyScope) {
                        scopeGraph->addScopeSucc(nextScope);
                    }

                    // Next block, loop body or successor block
                    ScopeInfo si(scopeGraph->getCurrScope(), nextScope, loopStack);
                    blockSuccs.push_back({succBlock, si});
                }
                
                // Put null statement to ensure non-empty scope to exit from
                // printCurrentScope() by level, required for sequential loops
                scopeGraph->storeNullStmt(scopeGraph->getCurrScope());
                
                // Add IF statement else scope after then scope
                if (elseScope.get()) {
                    scopeGraph->addScopeSucc(elseScope);
                }
                
                if (!loopVisited && isNotLibrarySpace) statTerms.insert(term);
                
            } else
            if ( isa<BreakStmt>(term) ) {
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 1, 
                                 "No one successor in break");
                
                // Parse break statement, @termCondValue not used
                auto stmtStr = parseTerm(term, termCValue);
                bool isLoopBreak = !stmtInfo.isSwitchBreak(term);
                
                // Remove last loop from stack for loop break
                if (isLoopBreak) {
                    SCT_TOOL_ASSERT (!loopStack.empty(), 
                                     "Loop stack is empty in break");
                    auto& currLoop = loopStack.back();
                    
                    // Do not write @break for removed loop and switch statement
                    if (!currLoop.removed) {
                        scopeGraph->storeStmt(term, stmtStr.getValue());
                    } else {
                        // Add empty string to have statement for break in the scope
                        scopeGraph->storeStmt(term, string());
                        // Removed loop break branch is run in separate analysis
                        breakInRemovedLoop = true;
                    }
                    loopStack.pop_back();
                }
                
                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                
                if (!breakInRemovedLoop) {
                    // Next block is outside of the loop, so it at least one level up
                    auto nextScope = getScopeForBlock(succBlock, level-1);
                    scopeGraph->addScopeSucc(nextScope);

                    ScopeInfo si(scopeGraph->getCurrScope(), nextScope, loopStack);
                    blockSuccs.push_back({succBlock, si});
                }
                if (isNotLibrarySpace) statTerms.insert(term);
                
            } else
            if ( isa<ContinueStmt>(term) ) {
                SCT_TOOL_ASSERT (cfgBlock->succ_size() == 1, 
                                 "No one successor in continue");
                SCT_TOOL_ASSERT (!loopStack.empty(), 
                                 "Loop stack is empty in continue");

                // Parse and store continue statement, @termCondValue not used
                auto stmtStr = parseTerm(term, termCValue);
                
                // Generate @continue statement string
                auto& currLoop = loopStack.back();

                // Do not write @continue for removed loop
                if (!currLoop.removed) {
                    scopeGraph->storeStmt(term, stmtStr.getValue());
                } else {
                    // Add empty string to have statement for continue in the scope
                    scopeGraph->storeStmt(term, string());
                    // Removed loop continue branch is run in separate analysis
                    contInRemovedLoop = true;
                }

                CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
                AdjBlock succBlock(*iter);
                
                if (!contInRemovedLoop) {
                    // Next block is inside of the loop, so it at least one level up
                    auto nextScope = getScopeForBlock(succBlock, level-1);
                    scopeGraph->addScopeSucc(nextScope);

                    ScopeInfo si(scopeGraph->getCurrScope(), nextScope, loopStack);
                    blockSuccs.push_back({succBlock, si});
                }
                if (isNotLibrarySpace) statTerms.insert(term);
                
            } else    
            if (isa<GotoStmt>(term)) {
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
                auto nextScope = getScopeForBlock(succBlock, level);
                // Set @nextScope as successor of the current scope
                scopeGraph->addScopeSucc(nextScope);

                // Level not changed in general block, level up provided
                // for next block if it has multiple inputs 
                ScopeInfo si(scopeGraph->getCurrScope(), nextScope, loopStack);
                blockSuccs.push_back({succBlock, si});
                
            } else 
            if (cfgBlock->succ_size() > 1) {
                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Block " << cfgBlock->getBlockID() << " has " 
                         << block.getCfgBlock()->succ_size() << " successors" << endl;
                }
                SCT_TOOL_ASSERT (false, "Too many successors");

            } else {
                // Do nothing if no successor, exit block has no successors
                noExitFunc = false;
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
            unsigned blockID = bs.first.getCfgBlockID();
            
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
            // Insert new scope according with CFG block ID, higher ID goes first
            if (!found) {
                auto j = delayed.begin();
                for (; j != delayed.end(); ++j) {
                    if (blockID < j->first.getCfgBlockID()) {
                        break;
                    }
                }
                // Insert new scope before element pointed by @j
                delayed.insert(j, {bs.first, vector<ScopeInfo>(1, bs.second)});

                if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                    cout << "Add to delayed new Block B" << AdjBlock::getCfgBlock(bs.first)->getBlockID() 
                         << "delayed size is " << delayed.size() << endl;
                }
            }
        }
        
        skipOneElement = false;
        skipOneBlock = false;
        
        // Store context in @break/@continue and run separate analysis
        // for @break/@continue, these flags not set after wait
        // Return to stored context after @break/@continue finished
        if (breakInRemovedLoop || contInRemovedLoop) {
            lastContext = std::make_shared<ScFuncContext>(
                CfgCursor(funcDecl, block.getCfgBlock(), 0),
                returnValue, modval, recval, delayed, loopStack, 
                calledFuncs, true, noExitFunc, scopeGraph, 
                codeWriter->serialize());
            
            string stmtName = ((breakInRemovedLoop) ? "break" : "continue");
            if (DebugOptions::isEnabled(DebugComponent::doGenBlock)) {
                cout << "--------------------------------------" << endl;
                cout << "Start analysis in "<< stmtName << " (removed loop)" << endl;
                cout << term->getSourceRange().getBegin().printToString(sm) << endl;
                cout << "--------------------------------------" << endl;
                cout << "block #" << block.getCfgBlockID() << endl;
                //contextStack.printCursorStack();
            }

            // Store current context stack to analyze it after break/continue done
            contextStack.push_back( *(lastContext.get()) );
            ScProcContext currContextStack = contextStack;
            storedContextStacks.push_back(currContextStack); // TODO: add std::move
            lastContext = nullptr;

            // Clone context to have separate scope graphs connected in chain
            contextStack = contextStack.clone(inMainLoop);
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
            CFGBlock::succ_iterator iter = cfgBlock->succs().begin();
            AdjBlock succBlock(*iter);
            block = succBlock;
            deadCond = false;
            
            continue;
        }
        
        // Get next block from @delayed
        if (!delayed.empty()) {
            // Take block from @delayed
            auto i = delayed.rbegin();
            
            // Prepare next block
            prepareNextBlock(i->first, i->second);

            // Remove block from @delayed, use ++ as @base() points to next object 
            delayed.erase((++i).base());

        } else {
            // No more blocks to analysis 
            if (contextStack.empty()) {

                // Exit if no more context to analyzes
                if (storedContextStacks.empty()) { 
                    break;
                }

                // Get next stored context stack
                contextStack = storedContextStacks.back();
                storedContextStacks.pop_back();
            }
            
            // Restore callee function context, skip function call element in block
            bool noExitFuncLast = noExitFunc; 
            bool breakContext = restoreContext(true);
            skipOneElement = !breakContext;
            // Skip block break/continue in removed loop
            // Skip block with function call if there is no return from function
            skipOneBlock = breakContext || noExitFuncLast;
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
        //cout << "var " << var << " pval " << pval << endl;
        
        // If variable has another parent it can be in MIF/record array,
        // collect MIF/record array sizes to add in reset section declaration
        string indx;
        if (pval != modval) {
            // Get variable for @pval or its derived class
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

void ScTraverseProc::setConstEvalFuncs(const std::unordered_map<
                                       CallStmtStack, SValue>& funcs) 
{
    for (const auto& entry : funcs) {
        // Skip NO_VALUE first as it could be for non-function call
        if (!entry.second.isInteger()) continue;
        
        SCT_TOOL_ASSERT (!entry.first.empty(), "Empty call stack");
        auto callStmt = entry.first.back(); 
        auto callExpr = dyn_cast<CallExpr>(callStmt);
        
        if (!callExpr) {
            SCT_INTERNAL_ERROR (callStmt->getBeginLoc(), "No call expression found");
        }
        // Skip functions with @wait()
        auto funcDecl = callExpr->getDirectCallee();
        if (hasWaitFuncs.count(funcDecl) != 0) continue;

        constEvalFuncs.insert(entry);
    }
}

/// Report lack/extra sensitive to SS channels
void ScTraverseProc::reportSctChannel(sc_elab::ProcessView procView,
                                      const clang::FunctionDecl* funcDecl) 
{
    using namespace sc_elab;
    
    // Skip process if it is inside SS channel
    ModuleMIFView parent = procView.getParentModuleOrMIF();
    
    if (!isSctChannelSens(parent.getType(), funcDecl)) {
        std::unordered_set<sc_elab::ObjectView> sensSctChannels;
        for (auto sensEvent : procView.staticSensitivity()) {

            ArrayElemObjWithIndices source{sensEvent.sourceObj};
            source = source.obj.getAsArrayElementWithIndicies();

            ModuleMIFView parent = source.obj.getParentModuleOrMIF();
            if ( isSctChannelSens(parent.getType(), nullptr) ) {
                //std::cout << "  ADD to sens " << parent.getDebugString() << "\n";
                sensSctChannels.insert(parent);
            }
        }
        
        std::unordered_set<sc_elab::ObjectView> usedChannels;
        for (ObjectView obj : usedSctChannels) {
            if (skipSctTargets.count(obj) == 0) {
                usedChannels.insert(obj);
            }
        }

        for (ObjectView obj : sensSctChannels) {
            if (usedChannels.count(obj) == 0) {
                ScDiag::reportScDiag(funcDecl->getBeginLoc(), 
                    ScDiag::SYNTH_EXTRA_SENSTIV_THREAD) << obj.getDebugString();
            }
            //std::cout << "  sens " <<obj.getDebugString() << "\n";
        }

        for (ObjectView obj : usedChannels) {
            if (sensSctChannels.count(obj) == 0) {
                ScDiag::reportScDiag(funcDecl->getBeginLoc(), 
                    ScDiag::SYNTH_NON_SENSTIV_THREAD) << obj.getDebugString();
            }
            //std::cout << "  used " <<obj.getDebugString() << "\n";
        }
    }
}

}