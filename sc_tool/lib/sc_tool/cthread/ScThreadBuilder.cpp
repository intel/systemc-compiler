/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 * Modified by: Mikhail Moiseev
 */

#include "ScThreadBuilder.h"

#include "sc_tool/cfg/ScStmtInfo.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/cthread/ScCFGAnalysis.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/ScCommandLine.h"
#include "sc_tool/utils/BitUtils.h"
#include "sc_tool/utils/VerilogKeywords.h"
#include <llvm/Support/Debug.h>
#include <sstream>

using namespace llvm;
using namespace clang;
using namespace sc_elab;

namespace sc
{

const clang::CFG *
ThreadBuilder::getCurrentCFG(const sc::CfgCursorStack &callStack,
                                 const SValue & dynModVal)
{
    return cfgFab.get(getCurrentFuncDecl(callStack, dynModVal));
}

const clang::FunctionDecl *
ThreadBuilder::getCurrentFuncDecl(const CfgCursorStack &callStack,
                                      const SValue & dynModVal)
{
    if(callStack.empty()) {
        return entryFuncDecl;
    } else {

        clang::FunctionDecl * funcDecl;
        if (auto callExpr = getCursorCallExpr(callStack.back()))
            funcDecl = const_cast<FunctionDecl *>(callExpr->getDirectCallee());
        else
            funcDecl = callStack.back().getFuncDecl();

        bool isVirtual = false;
        if (auto *methodDecl = dyn_cast<CXXMethodDecl>(funcDecl))
            isVirtual = methodDecl->isVirtual();

        if (isVirtual)
            return getVirtualFunc(dynModVal, funcDecl).second;

        return funcDecl;
    }
}

const clang::CallExpr *
ThreadBuilder::getCursorCallExpr(const CfgCursor &callCursor)
{
    auto el = (*callCursor.getBlock())[callCursor.getElementID()];
    if (auto cfgStmt = el.getAs<CFGStmt>()) {
        const Stmt *stmt = cfgStmt->getStmt();
        return dyn_cast<CallExpr>(stmt);
    }

    return nullptr;
}

// Fill array dimensions for local variable/local record field array
// Input can be local array variable, field variable which can be 
// record array element or other non-array variable
clang::QualType getLocalArrayDims(const SValue& val, IndexVec& arrayDims)
{
    using std::cout; using std::endl;
    //cout << "getLocalArrayDims val " << val << " : ";
    
    SValue mval = val;
    bool locVarArray = false;
    bool locRecField = false;
  
    // For field of record array
    if (val.isVariable()) {
        SValue parent = val.getVariable().getParent();
        
        if (parent.isRecord() && parent.getRecord().isLocal()) {
            mval = parent.getRecord().var;
            locRecField = true;
            //cout << "parent " << parent << " mval " << mval << " locRecField " << locRecField << endl;
        }
    }

    // Get dimensions from variable type
    clang::QualType rtype = mval.getType();

    if (locRecField) {
        while (rtype->isConstantArrayType()) {
            auto* cArrayType = dyn_cast<ConstantArrayType>(rtype);
            arrayDims.push_back(cArrayType->getSize().getZExtValue());
            rtype = cArrayType->getElementType();
        }
    }
    
    // Get dimensions from variable array
    clang::QualType mtype = val.getType();

    while (mtype->isConstantArrayType()) {
        auto* cArrayType = dyn_cast<ConstantArrayType>(mtype);
        arrayDims.push_back(cArrayType->getSize().getZExtValue());
        mtype = cArrayType->getElementType();
        locVarArray = true;
    }
    
    //for (auto i : arrayDims) cout << i << " ";
    //cout << endl;
    
    // Return field type for record field
    return (locVarArray ? mtype : locRecField ? val.getType() : rtype);
}

sc_elab::VerilogProcCode ThreadBuilder::run()
{
    using std::cout; using std::endl; using std::hex; using std::dec;

    // Run constant propagation for all states
    if (DebugOptions::isEnabled(DebugComponent::doConstStmt) ||
        DebugOptions::isEnabled(DebugComponent::doConstTerm) ||
        DebugOptions::isEnabled(DebugComponent::doConstLoop) ||
        DebugOptions::isEnabled(DebugComponent::doConstCfg)) {
        std::cout << endl << "--------------- THREAD GLOBAL CPA : " 
                  << entryFuncDecl->getNameAsString() << endl;
    }
    
    auto parentModView = procView.getParentModule();
    auto verMod   = elabDB.getVerilogModule(parentModView);
    bool hasReset = !procView.resets().empty();
    bool isMethod = procView.isScMethod();

    // Preliminary CPA
    std::unordered_set<SValue> defVals;
    DebugOptions::suspend();
    auto preState = std::shared_ptr<ScState>(globalState->clone());
    ScTraverseConst preConst(astCtx, preState, modSval, 
                            globalState, &elabDB, &threadStates, 
                            &findWaitVisitor, false, isMethod);
    preConst.setHasReset(hasReset);
    preConst.run(verMod, entryFuncDecl);
    
    for (const auto& entry : preConst.getWaitStates()) {
        for (const auto& sval : entry.second.getDefArrayValues()) {
            defVals.insert(sval);
        }
    }

    // Remove defined member variables from state
    globalState->removeDefinedValues(defVals);
    DebugOptions::resume();
    
    // Run global CPA, used to provide initial state for local CPA
    std::shared_ptr<ScState> globalStateClone(globalState->clone());
    travConst = std::make_unique<ScTraverseConst>(astCtx,
                            globalStateClone, modSval, 
                            globalState, &elabDB, &threadStates, 
                            &findWaitVisitor, false, isMethod);
    travConst->setHasReset(hasReset);
    travConst->run(verMod, entryFuncDecl);
    
    // Check for empty process w/o @wait`s, return empty process code
    if (travConst->getLiveStmts().empty()) {
        return VerilogProcCode(true);
    }
    
    // Check at least one @wait achieved
    if (travConst->getWaitStates().empty()) {
        ScDiag::reportScDiag(entryFuncDecl->getBeginLoc(),
                             ScDiag::SC_FATAL_THREAD_NO_STATE);
        SCT_INTERNAL_FATAL_NOLOC ("No states found for thread");
    }
    
    // Remove unused variable definition statements and their declarations
    if (REMOVE_UNUSED_VAR_STMTS()) {
        // Delete unused statements
        travConst->removeUnusedStmt();
        // Delete values from UseDef  which related to  unused statement removed 
        std::unordered_set<SValue> defVals = travConst->getDefinedVals();
        std::unordered_set<SValue> useVals = travConst->getUsedVals();
        for (auto& entry : travConst->getWaitStates()) {
            entry.second.filterUseDef(defVals, useVals);
        }
    }
    
    // Filter @wait`s from live statements
    bool emptyLiveStmts = true;
    for (Stmt* stmt : travConst->getLiveStmts()) {
        if (CallExpr* expr = dyn_cast<CallExpr>(stmt)) {
            FunctionDecl* funcDecl = expr->getDirectCallee();
            if (isWaitDecl(funcDecl)) continue;
        }
        emptyLiveStmts = false; break;
    }

    // Check for empty process with @wait`s, return empty process code
    if (emptyLiveStmts) {
        return VerilogProcCode(true);
    }
    
//    bool first = true;
//    for (auto entry : travConst->getConstEvalFuncs()) {
//        if (!entry.second.isInteger()) continue;
//        
//        if (first) {
//            cout << "THREAD " << entryFuncDecl->getNameAsString() << endl;
//            cout << "   functions evaluated as constant :" << endl;
//        }
//
//        auto callExpr = dyn_cast<const CallExpr>(entry.first.back());
//        cout << "    " << callExpr->getDirectCallee()->getNameAsString() 
//             << " " << entry.second << endl;
//    }
    
    // Fill UseDef for all states
    std::unordered_set<SValue> useVals;
    defVals.clear();
    for (const auto& entry : travConst->getWaitStates()) {
        analyzeUseDefResults(&entry.second, useVals, defVals);
    }
    
    // Clear initialization values for modified member variables
    clearDefinedVars(defVals);
    // Get non-modified member variables to avoid error reporting
    auto memInitVals = getInitNonDefinedVars(useVals, defVals);
    
    // Report read not initialized warning
    auto notInitReport = [&](const InsertionOrderSet<SValue>& values, 
                             ScDiag::ScDiagID diagId) 
    {
        for (auto val : values) {
            // Skip constants, channels and pointers to channel
            QualType type = val.getType();
            if (type.isConstQualified() || isConstReference(type) || 
                isPointerToConst(type) ||
                isScChannel(type) || isScVector(type) || 
                isScChannelArray(type) || isScToolCombSignal(type)) continue;
            
            // Skip null pointer
            if (isPointer(type)) {
                SValue rval = globalState->getValue(val);
                if (rval.isInteger() && rval.getInteger().isZero()) {
                    continue;
                }
            }
            // Skip member variables generated as @localparam
            if (memInitVals.count(val) != 0) continue;

            std::string varName = val.isVariable() ? 
                                  val.getVariable().asString(false) : "---";
            // Do not check duplicates
            ScDiag::reportScDiag(entryFuncDecl->getBeginLoc(), 
                                 diagId, false) << varName;
        }
    };
    
    // Report warning for read not initialized at reset section
    if (hasReset) {
        auto i = travConst->getWaitStates().find(0);
        if (i != travConst->getWaitStates().end()) {
            const ScState& resetState = i->second;

            InsertionOrderSet<SValue> values;
            for (const auto& val : resetState.getReadNotInitValues()) {
                // Do not report warning for record fields
                if (!resetState.isRecField(val)) values.push_back(val);
            }

            notInitReport(values, ScDiag::CPP_READ_NOTDEF_VAR_RESET);
        }
    }
    
    // Report warning for read not initialized in the same cycle where declared
    for (const auto& entry : travConst->getWaitStates()) {
        // Skip reset section to avoid duplicate warnings
        if (hasReset && entry.first == 0) continue;
        const ScState& waitState = entry.second;

        InsertionOrderSet<SValue> values;
        for (const auto& val : waitState.getReadNotInitValues()) {
            // Do not report warning for record fields
            if (!waitState.isRecField(val)) values.push_back(val);
        }

        notInitReport(values, ScDiag::CPP_READ_NOTDEF_VAR);
    }
    // Report warning for read and never initialized in the following cycles 
    notInitReport(threadReadNotDefVars, ScDiag::CPP_READ_NOTDEF_VAR);
    threadReadNotDefVars.clear();

    // Pass over all states, if first statement after wait() is infinite loop
    // and it is the same for all wait() the thread is single thread
    // The single state process must have reset
    auto mainLoopStmt = travConst->getMainLoopStmt();
    if (!mainLoopStmt) {
        ScDiag::reportScDiag(entryFuncDecl->getBeginLoc(),
                             ScDiag::SC_CTHREAD_NO_MAIN_LOOP);
    }

    // Single state thread, i.e. w/o state variable
    isSingleState = travConst->isSingleState();
    
    // Do not see any example with break/continue in removed loop for 
    // single state CTHREAD, but keep that for safety
    if (isSingleState && travConst->isBreakInRemovedLoop()) {
        SCT_INTERNAL_ERROR(procView.getLocation().second->getBeginLoc(),
                "Break/continue in removed loop for single state CTHREAD");
    }
    
    // Consider multi-states, wait in the middle and code before main loop
    if (!hasReset && travConst->codeBeforeMainLoop()) {
        ScDiag::reportScDiag(procView.getLocation().second->getBeginLoc(), 
                             ScDiag::SYNTH_NO_RESET_VIOLATION);
    }

    // Fill verVarTrais and put defined non-channel variables to process 
    // variable storage to be generated before the process
    generateThreadLocalVariables();

    //globalState->print();
    
    // Name generator with all member names for current module
    UniqueNamesGenerator& nameGen = verMod->getNameGen();

    std::unique_ptr<ScVerilogWriter> procWriter = std::make_unique<ScVerilogWriter>(
                astCtx.getSourceManager(), false, globalState->getExtrValNames(),
                nameGen, globalState->getVarTraits(), globalState->getWaitNVarName());

    // First traverse process stage and detecting duplicated states
    {
        //cout << " ==================== 1st stage ====================" << endl;
        travProc = std::make_unique<ScTraverseProc>(
                        astCtx, std::make_shared<ScState>(*globalState), modSval, 
                        procWriter.get(), &threadStates, &findWaitVisitor, 
                        false, isSingleState);

        travProc->setHasReset(!procView.resets().empty());
        travProc->setMainLoopStmt(travConst->getMainLoopStmt());
        travProc->setTermConds(travConst->getTermConds());
        travProc->setLiveStmts(travConst->getLiveStmts());
        travProc->setLiveTerms(travConst->getLiveTerms());
        travProc->setRemovedArgExprs(travConst->getRemovedArgExprs());
        travProc->setWaitFuncs(travConst->getWaitFuncs());
        travProc->setConstEvalFuncs(travConst->getConstEvalFuncs());
        
        // Traverse context stack, used to run TraverseProc
        traverseContextMap[RESET_ID] = ScProcContext();
        generateVerilogForState(RESET_ID);

        auto waitStatesVec = travConst->getWaitStatesInOrder();
        for (WaitID waitID : waitStatesVec) {
            generateVerilogForState(waitID);
            if (isSingleState) break;
        }
        
        // Prepare replaced states for second stage 
        if (!isSingleState) {
            //cout << "Replaced: " << endl;
            for (auto i = waitStatesVec.begin(); i != waitStatesVec.end(); ++i) {
                if (replacedStates.find(*i) != replacedStates.end()) continue;

                for (auto j = i+1; j != waitStatesVec.end(); ++j) {
                    if (replacedStates.find(*j) != replacedStates.end()) continue;
                    if (stateCodeMap[*i] != stateCodeMap[*j]) continue;
                    replacedStates.emplace(*j, *i);
                    //cout << "  " << *j << " " << *i << endl;
                }
            }
            SCT_TOOL_ASSERT (waitStatesVec.size() > replacedStates.size(),
                             "Incorrect replaced states number");
        }
    }

    // Generate state variable declaration
    generateThreadStateVariable();
    
    // Second traverse process stage, final code generation w/o duplicated states
    if (!isSingleState) {
        //cout << " ==================== 2nd stage ====================" << endl;
        // Clear before second stage
        procWriter->clear();
        traverseContextMap.clear();
        stateCodeMap.clear();
        
        travProc = std::make_unique<ScTraverseProc>(
                        astCtx, std::make_shared<ScState>(*globalState), modSval, 
                        procWriter.get(), &threadStates, &findWaitVisitor, 
                        false, isSingleState);

        travProc->setHasReset(!procView.resets().empty());
        travProc->setMainLoopStmt(travConst->getMainLoopStmt());
        travProc->setTermConds(travConst->getTermConds());
        travProc->setLiveStmts(travConst->getLiveStmts());
        travProc->setLiveTerms(travConst->getLiveTerms());
        travProc->setRemovedArgExprs(travConst->getRemovedArgExprs());
        travProc->setWaitFuncs(travConst->getWaitFuncs());
        travProc->setConstEvalFuncs(travConst->getConstEvalFuncs());
        // Set replaced states for second stage
        travProc->setReplacedStates(replacedStates);
        
        // Traverse context stack, used to run TraverseProc
        traverseContextMap[RESET_ID] = ScProcContext();
        generateVerilogForState(RESET_ID);

        auto waitStatesVec = travConst->getWaitStatesInOrder();
        for (WaitID waitID : waitStatesVec) {
            // Skip replaced state
            if (replacedStates.find(waitID) != replacedStates.end()) continue;

            generateVerilogForState(waitID);
            if (isSingleState) break;
        }
    }
    
    bool noneZeroElmntMIF = travConst->isNonZeroElmtMIF();
    VerilogProcCode procCode = getVerilogCode(isSingleState);

    // Skip MIF non-zero elements to get number of unique statements
    if (!noneZeroElmntMIF) {
        procCode.statStmtNum = travProc->statStmts.size();
        procCode.statTermNum = travProc->statTerms.size();
        procCode.statAsrtNum = travProc->statAsrts.size();
        procCode.statWaitNum = travProc->statWaits.size();
    }

    // Report error for lack/extra sensitive to SS channels
    travProc->reportSctChannel(procView, entryFuncDecl);
    
    return procCode;
}

// Fill @traverseContextMap and return reachable wait IDs
void ThreadBuilder::generateVerilogForState(WaitID stateID)
{
    SCT_TOOL_ASSERT (traverseContextMap.count(stateID), 
                     "No context ready for next state code generation");
    
    auto startTraverseContext = traverseContextMap.at(stateID);
    auto waitProcContexts = generateCodePath(startTraverseContext, stateID);

    for (auto &waitCtx : waitProcContexts) {
        traverseContextMap[waitCtx.first] = waitCtx.second;
    }
}

std::vector<std::pair<WaitID, ScProcContext>>
ThreadBuilder::generateCodePath(ScProcContext traverseContext, 
                                WaitID startStateID)
{
    using namespace std;

    auto threadDecl = threadStates.getEntryFunction();
    if (traverseContext.empty()) {
        travProc->setFunction(threadDecl);
    } else {
        travProc->setContextStack(traverseContext);
    }
    
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << "Start at wait #" << startStateID << endl;
    }
    
    if (DebugOptions::isEnabled(DebugComponent::doGenStmt) ||
        DebugOptions::isEnabled(DebugComponent::doGenTerm) ||
        DebugOptions::isEnabled(DebugComponent::doGenBlock) ||
        DebugOptions::isEnabled(DebugComponent::doGenRTL) ||
        DebugOptions::isEnabled(DebugComponent::doGenCfg)) {
        cout << endl << "--------------- THREAD GEN : " 
                        << threadDecl->getNameAsString() << endl;
        cout << "Run process in module " << modSval << endl;
    }
    
    // Use @ScTraverseConst results if this line commented
    travProc->run();
    
    // Print function RTL
    std::stringstream ss;
    travProc->printFunctionBody(ss);
    std::string generatedCode = ss.str();
    
    if (DebugOptions::isEnabled(DebugComponent::doGenRTL)) {
        cout << "---------------------------------------" << endl;
        cout << " Function body RTL: " << endl;
        cout << "---------------------------------------" << endl;
        travProc->printFunctionBody(cout);
        cout << "---------------------------------------" << endl;
    }
    
    // Fill wait contexts
    vector<pair<WaitID, ScProcContext>> waitContexts;
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << "Reached wait`s :" << endl;
    }

    for (auto& ctxStack : travProc->getWaitContexts()) {
        WaitID waitId = threadStates.getStateID(ctxStack.getCursorStack());
        waitContexts.push_back( {waitId, ctxStack} );
        
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "   wait #" << waitId << endl;
        }
    }
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << endl;
    }

    if (startStateID == RESET_ID && !procView.resets().empty()) {
        // Generate local variables of reset section
        std::stringstream ls;
        travProc->printResetDeclarations(ls);
        generatedCode = ls.str() + generatedCode;
    }
    
    SCT_TOOL_ASSERT(stateCodeMap.count(startStateID) == 0, 
                    "State code already generated");
    stateCodeMap[startStateID] = generatedCode;

    return waitContexts;
}


sc_elab::VerilogProcCode ThreadBuilder::getVerilogCode(bool isSingleState)
{
    std::string localVars;
    std::string caseBody;
    std::string resetSection;

    {
        // Generate Local Variables
        std::stringstream ls;
        travProc->printLocalDeclarations(ls);
        
        // Add wait(n) counter current to next assignment
        if (threadStates.hasWaitNState()) {
            ls << "    " << waitNRegNames.second << " = " 
                         << waitNRegNames.first << ";\n";
        }
        localVars = ls.str();
    }

    {
        // Generate Reset State
        std::string verOutStr;
        llvm::raw_string_ostream verRawOS{verOutStr};
        RawIndentOstream vout{verRawOS};

        if (!procView.resets().empty()) {
            vout.pushF("    ");
            vout << "begin\n";
            
            vout << stateCodeMap[RESET_ID];

            std::stringstream rs;
            travProc->printInitLocalInReset(rs);
            vout << rs.str();
                    
            // Add wait(n) counter initialization in reset, not required
            // if WAIT_N counter initialized in reset when first wait is wait(N)
            if (threadStates.hasWaitNState() && !threadStates.isFirstWaitN()) {
                vout << "    " << waitNRegNames.first << " <= 0;\n";
            }
        }

        resetSection = verRawOS.str();
    }

    {
        // Generate @case(PROC_STATE) ...
        std::string verOutStr;
        llvm::raw_string_ostream verRawOS{verOutStr};
        RawIndentOstream vout{verRawOS};

        if (!isSingleState) {
            vout.pushF("    ");
            vout << "    " << stateRegNames.second << " = " 
                           << stateRegNames.first << ";\n";
            vout << "\n";

            vout << "case (" << stateRegNames.first << ")\n";
            vout.pushF("    ");
        }

        for (size_t waitID = 0; waitID < threadStates.getNumFSMStates(); ++waitID) 
        {
            if (replacedStates.find(waitID) != replacedStates.end()) continue;

            if (!isSingleState) {
                vout << waitID << ": begin\n";
            }

            vout << stateCodeMap[waitID];

            if (!isSingleState) {
                vout << "end\n";
            }
        }

        if (!isSingleState) {
            vout.popF();
            vout << "endcase\n";
            vout.popF();
        }

        caseBody = verRawOS.str();
    }
    
    // Get temporal assertions
    std::stringstream ssr;
    travProc->printTemporalAsserts(ssr, true);
    std::string tempRstAsserts = ssr.str();
    std::stringstream ss;
    travProc->printTemporalAsserts(ss, false);
    std::string tempAsserts = ss.str();
    
    //std::cout << "Temporal asserts in proc:\n" << tempAsserts << std::endl;

    return sc_elab::VerilogProcCode(caseBody, localVars, resetSection, 
                                    tempAsserts, tempRstAsserts);
}


void ThreadBuilder::analyzeUseDefResults(const ScState* finalState, 
                                         std::unordered_set<SValue>& useVals,
                                         std::unordered_set<SValue>& defVals)
{
    using namespace std;
    //  Analyzing Use-Def analysis results
    //
    //  SystemC channels:
    //   - Channel Read  -> Do nothing
    //   - Channel Write -> Create next-state variable
    //
    //  Plain data members:
    //   - Constant read              -> Do nothing
    //   - Read before def            -> Create register & next-state variable
    //   - Defined before read        -> Create combinational variable

    using std::cout; using std::endl;
    DEBUG_WITH_TYPE(DebugOptions::doThreadBuild,
        outs() << "STATE WITH UseDef\n"; finalState->print(); );
    
//    globalState->print();
//    finalState->print();
//    auto defined = finalState->getDefArrayValues();
//    cout << endl << "------- Defined filtered ------ " << endl;
//    for (auto& i : defined) {
//        cout << "   " << i << endl;
//    }
//    cout << "-------------------------------- " << endl;
       
    // ReadNotDefined combinational signals with clear flag
    std::unordered_set<SValue>  combSignClearRnDs; 
        
    // Process read non-constant, non-channel variables
    //cout << "READ: " << endl;
    for (const auto& val : finalState->getReadValues()) {
        //cout << "   " << val << endl;
        
        if (!val.isVariable() && !val.isObject()) {
            SCT_TOOL_ASSERT (false, "Unexpected UD value type");
            continue;
        }
        useVals.insert(val);
        
        // Skip constants, channels and pointers to channel
        if (!val.getType().isConstQualified() &&
            !isPointerToConst(val.getType()) &&
            !isScChannel(val.getType()) && 
            !isScVector(val.getType()) &&
            !isScChannelArray(val.getType()) &&
            !isScToolCombSignal(val.getType())) continue;

        // Skip null pointer
        if (isPointer(val.getType())) {
            SValue rval = globalState->getValue(val);
            if (rval.isInteger() && rval.getInteger().isZero())
                continue;
        }

        threadReadVars.insert(val);
    }
    
    // Process read-not-defined
    //cout << "READ_NOT_DEF: " << endl;
    for (const auto& val : finalState->getReadNotDefinedValues()) {
        //cout << "   " << sval << endl;

        if (!val.isVariable() && !val.isObject()) {
            SCT_TOOL_ASSERT (false, "Unexpected UD value type");
            continue;
        }
        
        // Check @sc_comb_sig read before defined, report error if so
        QualType qtype = val.getType();
        if (isScToolCombSignal(qtype)) {
            // @sc_comb_sig has second argument @CLEAR
            bool sctCombSignClear = false;
            if (auto clearArg = getTemplateArgAsInt(qtype, 1)) {
                sctCombSignClear = !clearArg->isZero();
            }

            // Report fatal error as it generates incorrect code
            if (sctCombSignClear) {
                combSignClearRnDs.insert(val);
            }
        }

        // Last condition to check record fields in channels
        if (isScChannel(val.getType()) || isScVector(val.getType()) ||
            isScChannelArray(val.getType()) || 
            isScToolCombSignal(val.getType()) ||
            val.getVariable().getParent().isScChannel())
        {
            // RnD channel becomes read only if it is not defined later
            if (!threadRegVars.count(val)) {
                threadReadOnlyVars.insert(val);
            }
            
        } else {
            
            bool isConsRef = val.isConstReference();
            bool isConsVal = !isConsRef && ScState::isConstVarOrLocRec(val);

            if (isConsVal || isConsRef) {
                // Member constant
                bool isMember = (bool)globalState->getElabObject(val);

                // Check if local constant has integer value to replace it or
                // defined in reset means its value available in all states 
                // Such local constant considered as read only, no register
                bool readOnlyConst = false;
                if (!isMember) {
                    readOnlyConst = isConsVal && 
                                    travConst->getResetDefConsts().count(val);
                }

                if (isMember || readOnlyConst) {
                    // Member and local replaced constant becomes read only
                    if (!threadRegVars.count(val) && 
                        !threadReadOnlyVars.count(val)) {
                        threadReadOnlyVars.insert(val);
                    }
                } else {
                    // Local not replaced constant becomes registers
                    if (!threadRegVars.count(val)) {
                        threadRegVars.insert(val); 
                    }
                    if (threadReadOnlyVars.count(val)) {
                        threadReadOnlyVars.erase(val);
                    }
                }
            } else {
                // Put variable into ReadNotDefined if meet it first time,
                // used to report error only
                if (!threadRegVars.count(val) && !threadCombVars.count(val)) {
                    // Do not report warning for record fields
                    if (!finalState->isRecField(val)) threadReadNotDefVars.insert(val);
                }
                // RnD variable becomes register
                if (threadCombVars.count(val)) {
                    threadCombVars.erase(val);
                }
                threadRegVars.insert(val);
            }
        }
    }

    // Processing define values
    //cout << "DEFINED: " << endl;
    for (const auto& val : finalState->getDefArrayValues()) {
        //cout << "   " << sval << endl;

        if (!val.isVariable() && !val.isObject()) {
            SCT_TOOL_ASSERT (false, "Unexpected UD value type");
            continue;
        }
        defVals.insert(val);
        
        if (combSignClearRnDs.count(val)) {
            ScDiag::reportScDiag(
                procView.getLocation().second->getBeginLoc(),
                ScDiag::SYNTH_COMB_SIG_READNOTDEF);
        }
         
        if (isScChannel(val.getType()) || isScVector(val.getType()) ||
            isScChannelArray(val.getType()) || 
            isScToolCombSignal(val.getType())) 
        {
            // Defined channel becomes register
            if (threadReadOnlyVars.count(val)) {
                threadReadOnlyVars.erase(val);
            }
            threadRegVars.insert(val);
            
        } else {
            
            bool isConsRef = val.isConstReference();
            bool isConsVal = !isConsRef && ScState::isConstVarOrLocRec(val);

            // Defined variable becomes combinational if it is not register
            if (!isConsVal && !isConsRef) {
                if (!threadRegVars.count(val)) {
                    threadCombVars.insert(val);
                }
                if (threadReadNotDefVars.count(val)) {
                    threadReadNotDefVars.erase(val);
                }
            }
        }
    }
    
    // Processing declared values to add unused but declared local variables
    // into @varTraits, to remove their declarations in VerilogWriter
    for (const auto& val : finalState->getDeclaredValues()) {
        // Skip array elements and other non-variables
        if (val.isVariable()) {
            if (!ScState::isConstVarOrLocRec(val)) {
                if (!threadRegVars.count(val)) {
                    threadCombVars.insert(val);
                }
            }
        }
    }
    
    
    /*if (!threadRegVars.empty()) {
        cout << endl << "threadRegVars :" << endl;
        for (auto v : threadRegVars) {
            cout << "   " << v << endl;
        }
        cout << "----------------" << endl;
    }

    if (!threadCombVars.empty()) {
        cout << "threadCombVars :" << endl;
        for (auto v : threadCombVars) {
            cout << "   " << v << endl;
        }
        cout << "----------------" << endl;
    }

    if (!threadReadOnlyVars.empty()) {
        cout << "threadReadOnlyVars :" << endl;
        for (auto v : threadReadOnlyVars) {
            cout << "   " << v << endl;
        }
        cout << "----------------" << endl;
    }*/
}

// Get initialized variables to skip report read-no-initialized error
std::unordered_set<SValue>
ThreadBuilder::getInitNonDefinedVars(const std::unordered_set<SValue>& useVals,
                                     const std::unordered_set<SValue>& defVals)
{
    using namespace std;
    auto parentModView = procView.getParentModule();
    auto verMod  = elabDB.getVerilogModule(parentModView);
    std::unordered_set<SValue> memInitVals;
    
    for (const SValue& val : useVals) {
        if (defVals.count(val) != 0) continue;
        
        // Class field must be in this map, no local variables here
        if (auto elabObj = globalState->getElabObject(val)) {
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }

            auto verVars = verMod->getVerVariables(*elabObj);
            
            for (auto* verVar : verVars) {
                if (verVar->isConstant()) {
                    memInitVals.insert(val); break;
                }
            }
        }
    }
    return memInitVals;
}

// Clear initialization for variables which has been defined  
void ThreadBuilder::clearDefinedVars(const std::unordered_set<SValue>& defVals)
{
    using namespace std;
    auto parentModView = procView.getParentModule();
    auto verMod  = elabDB.getVerilogModule(parentModView);
    
    for (const SValue& val : defVals) {
        // Class field must be in this map, no local variables here
        if (auto elabObj = globalState->getElabObject(val)) {
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }

            auto verVars = verMod->getVerVariables(*elabObj);
            
            for (auto* verVar : verVars) {
                verVar->clearInitVals();
            }
        }
    }
}

// Create field value for channel record
// \param fieldObj -- field view
// \param parent -- parent record channel
sc::SValue createRecChanField(ObjectView fieldObj, SValue parent) 
{
    auto* fieldDecl = fieldObj.getValueDecl();
    SCT_TOOL_ASSERT(fieldDecl, "No declaration for channel record field");
    SCT_TOOL_ASSERT(!fieldObj.isStatic() || fieldObj.isConstant(), 
                    "Static non-constant field in channel record");

    // Field with record channel as parent class
    return SValue(fieldDecl, parent);
}

void ThreadBuilder::generateThreadStateVariable()
{
    using std::cout; using std::endl;
    
    auto parentModView = procView.getParentModule();
    auto verMod  = elabDB.getVerilogModule(parentModView);
    std::string procName = *procView.getFieldName();

    {
        // Generate PROC_STATE variable
        if (!isSingleState) {
            size_t stateCount = threadStates.getNumFSMStates();
            size_t bitWidth = bitsForIndex(stateCount);

            auto procStateName = procName + "_PROC_STATE";
            auto procStateNextName = procName + "_PROC_STATE_next";

            auto* procStateVar= verMod->createProcessLocalVariable(
                                        procView, procStateName, bitWidth, 
                                        {}, false);
            auto* procStateNextVar = verMod->createProcessLocalVariable(
                                        procView, procStateNextName, bitWidth, 
                                        {}, false);
            verMod->procVarMap[procView].insert(procStateVar);
            verMod->procVarMap[procView].insert(procStateNextVar);                            

            stateRegNames = make_pair(procStateVar->getName(), 
                                      procStateNextVar->getName());

            globalState->setProcStateName(procStateVar->getName(), procStateNextVar->getName());
            verMod->addProcRegisterNextValPair(procView, procStateVar, procStateNextVar);

            verMod->addVarUsedInProc(procView, procStateVar, 0, 0);
            verMod->addVarDefinedInProc(procView, procStateVar, 0, 0);
            verMod->addVarUsedInProc(procView, procStateNextVar, 0, 0);
            verMod->addVarDefinedInProc(procView, procStateNextVar, 0, 0);
        }
    }
}

void ThreadBuilder::generateThreadLocalVariables()
{
    using std::cout; using std::endl;
    // Clear verVarTraits after previous process, required for channels
    globalState->clearVerilogTraits();
    
    auto parentModView = procView.getParentModule();
    auto verMod  = elabDB.getVerilogModule(parentModView);
    std::string procName = *procView.getFieldName();
    
    //cout << "--- proc #" << procView.getID() << " " << procName << " modval " << modSval << endl;
    //clang::CXXMethodDecl* threadDecl = procView.getLocation().second;
    //cout << (threadDecl ? threadDecl->getBeginLoc().printToString(sm) : "") << endl;
    
    // Do not generate variable declarations for non-zero elements of MIF array
    bool zeroElmntMIF = travConst->isZeroElmtMIF();
    bool noneZeroElmntMIF = travConst->isNonZeroElmtMIF();
    std::string mifElemSuffix = (zeroElmntMIF || noneZeroElmntMIF) ?
                                 travConst->getMifElmtSuffix() : "";
//    cout << "Thread is zeroElmntMIF " << zeroElmntMIF << " noneZeroElmntMIF " 
//         << noneZeroElmntMIF << " nonZeroSuffix " << mifElemSuffix << endl;
    
    {
        // Thread has wait(N), create counter variable
        if (threadStates.hasWaitNState()) {

            auto counterName = procName + "_WAIT_N_COUNTER";
            auto counterNextName = procName + "_WAIT_N_COUNTER_next";

            size_t bitWidth = bitsInNumber(threadStates.getMaxWaitN());

            // Create variable for counter in VerilogModule
            auto* counterVar = verMod->createProcessLocalVariable(
                                    procView, counterName, bitWidth, {}, false);
            auto* counterNextVar = verMod->createProcessLocalVariable(
                                    procView, counterNextName, bitWidth, {}, false);
            verMod->procVarMap[procView].insert(counterVar);
            verMod->procVarMap[procView].insert(counterNextVar);

            waitNRegNames = make_pair(counterVar->getName(), 
                                      counterNextVar->getName());
            
            // Add name for counter to state
            globalState->setWaitNVarName(counterVar->getName(), 
                                         counterNextVar->getName());
            verMod->addProcRegisterNextValPair(procView, counterVar, 
                                               counterNextVar);
            verMod->addVarUsedInProc(procView, counterVar, 0, 0);
            verMod->addVarDefinedInProc(procView, counterVar, 0, 0);
            verMod->addVarUsedInProc(procView, counterNextVar, 0, 0);
            verMod->addVarDefinedInProc(procView, counterNextVar, 0, 0);
        }
    }
    
    // @varTraits contains name for accessed variable/channel and the correspondent
    // variable/channel of zero element of MIF array. The accessed variable/channel
    // used in analysis of MIF array element process. The variable/channel 
    // of zero element of MIF array used in analysis of parent process accesses 
    // the member of MIF array element. Therefore double put into @varTraits.
                                 
    // ------------- Processing registers
    for (auto regVar : threadRegVars) {
        //cout << "Thread Reg: " << regVar << endl;

        // Ignore record value, record fields are used instead
        QualType varType = regVar.getType();
        if (isUserClass(getDerefType(varType))) continue;
        if (isZeroWidthType(getDerefType(varType))) continue;
        // Get array/vector element type, for array/vector of record channels
        if (varType->isArrayType()) {
            varType = QualType(varType->getArrayElementTypeNoTypeQual(), 0);
        } else {
            while (isScVector(varType)) {
                varType = *(getTemplateArgAsType(varType, 0));
            }
        }
        bool isRecordChan = (bool)isUserClassChannel(getDerefType(varType));
        
        // Replace value to array first element 
        // Use global state because only members needs to be processed here
        SValue regVarZero = globalState->getFirstArrayElementForAny(
                                        regVar, ScState::MIF_CROSS_NUM);
        bool isNonZeroElmt = regVar != regVarZero;
        SCT_TOOL_ASSERT (regVarZero, "No zero element found in global state");
        //cout << "   regVar " << regVar << " regVarZero " << regVarZero << " isNonZeroElemnt " << isNonZeroElmt << endl;
        
        // Get access place, access after reset includes access in SVA
        // Use zero value to consider some MIF array elements not used
        bool inResetAccess = travConst->isInResetAccessed(regVarZero);
        bool afterResetAcess = travConst->isAfterResetAccessed(regVarZero) ||
                               travConst->isSvaProcAccessed(regVarZero);
        auto accessPlace = VerilogVarTraits::getAccessPlace(inResetAccess, 
                                                            afterResetAcess);

        if (auto elabObj = globalState->getElabObject(regVarZero)) {
            // Module/class field
            //cout << "Convert to proc local: " << elabObj->getDebugString() << " id " << elabObj->getID() << endl;

            // Get first element for non-first array element
            if (!elabObj) {
                SCT_TOOL_ASSERT (false, "No elaboration object for variable "
                                        "in thread");
                continue;
            }
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }

            // @sct_comb_signal flag 
            bool sctCombSignal = false; 
            bool sctCombSignClear = false;
            bool sctClearSignal = false;
            
            if (elabObj->isChannel()) {
                QualType qtype = elabObj->getType();
                sctCombSignal = isScToolCombSignal(qtype);
                
                if (sctCombSignal) {
                    // @sc_comb_sig has second argument @CLEAR
                    if (auto clearArg = getTemplateArgAsInt(qtype, 1)) {
                        sctCombSignClear = !clearArg->isZero();
                    }
                } else {
                    sctClearSignal = isScToolClearSignal(qtype);
                }
                        
                // For channels and channel arrays process analyzer wants to
                // see ScChannel SValue of first (or singular) array element
                // instead of array/pointer SVariable
                while (!regVar.isScChannel() && !regVar.isUnknown()) {
                    regVar = globalState->getValue(regVar);
                }
                SCT_TOOL_ASSERT (regVar.isScChannel(), "No channel found");
                while (!regVarZero.isScChannel() && !regVarZero.isUnknown()) {
                    regVarZero = globalState->getValue(regVarZero);
                }
                SCT_TOOL_ASSERT (regVarZero.isScChannel(), "No channel found");
            }
            
            bool isNullPtr = false;
            bool isDanglPtr = false;

            if (isPointer(regVar.getType())) {
                SValue rval = globalState->getValue(regVar);
                isNullPtr = rval.isInteger() && rval.getInteger().isZero();
                isDanglPtr = rval.isUnknown();
            }
            
            // Module data members
            auto verVars = verMod->getVerVariables(*elabObj);
            bool isRecord = elabObj->isRecord();
            
            // Skip channels of record fields passed by reference
            if (verVars.empty() && regVarZero.isVariable()) {
                if (regVarZero.getVariable().getParent().isScChannel()) continue;
            }

            // Constant dangling/null pointer and record have no variable
            if (verVars.empty() && !isNullPtr && !isDanglPtr && !isRecord) {
                std::string err = "No register variable for elaboration object "+
                                  elabObj->getDebugString();
                if (regVarZero.isVariable()) {
                    SCT_INTERNAL_ERROR(regVarZero.getVariable().getDecl()->
                                       getBeginLoc(), err);
                } else {
                    SCT_INTERNAL_ERROR_NOLOC(err);
                }
            }
            
            bool isChannel = regVar.isScChannel();
            bool isConst = !isChannel && (regVar.getType().isConstQualified()||
                                          isPointerToConst(regVar.getType()));
            for (auto* verVar : verVars) {
                isConst = isConst || verVar->isConstant();
            }
            
            // Skip constant variable it is declared as @localparam
            if (isConst) {
                threadReadOnlyVars.insert(regVar);
                continue;
            }
            
            // Get record view from record channel
            ElabObjVec rcFields;
            //cout << "  isRecordChan " << isRecordChan << "\n";
            if (isRecordChan) {
                if (elabObj->isSignal()) {
                    SignalView signalView = *elabObj;
                    auto recView = signalView.getSignalValue().record();
                    recView->getFieldsNoZero(rcFields);
                } else 
                if (elabObj->isPrimitive()) {
                    PortView portView = *elabObj;
                    SignalView signalView(portView.getBindedSignal());
                    auto recView = signalView.getSignalValue().record();
                    recView->getFieldsNoZero(rcFields);
                }
                SCT_TOOL_ASSERT(rcFields.size(), "No record fields found");
            }
            
            // Loop required for record and record channel
            unsigned i = 0;
            for (auto* verVar : verVars) {
                bool first = i == 0;

                // Skip constant fields in record channel
                if (isRecordChan && rcFields[i].isConstant()) {
                    //cout << "verVar " << verVar->getName() << " isConst "
                    //     << rcFields[i].isConstant() << endl;
                    i++; continue;
                }
                
                // Add var <= var_next in @always_ff body
                if (sctCombSignal && sctCombSignClear) {
                    // Add to verVarTraits
                    if (first || isRecordChan) {
                        // Swap current and next name (next name is empty)
                        // Put suffix to @varTraits to use in @var_next = @var
                        SValue vval; 
                        vval = isRecordChan ? 
                               createRecChanField(rcFields[i], regVar) : regVar;
                        globalState->putVerilogTraits(vval,
                            VerilogVarTraits(VerilogVarTraits::COMBSIGCLEAR, 
                            accessPlace, true, isNonZeroElmt, false,
                            verVar->getName(), std::optional<std::string>(""),
                            mifElemSuffix));
                        
                        vval = isRecordChan ? 
                               createRecChanField(rcFields[i], regVarZero) : regVarZero;
                        globalState->putVerilogTraits(vval,
                            VerilogVarTraits(VerilogVarTraits::COMBSIGCLEAR, 
                            accessPlace, true, !isNonZeroElmt, false,
                            verVar->getName(), std::optional<std::string>(""),
                            mifElemSuffix));
                    }
                } else {
                    std::string nextName = verVar->getName();
                    if (VerilogKeywords::check(nextName)) {
                        nextName += "_v";
                    }
                    nextName += "_next";
                    
                    // Do not change name for non-zero elements of MIF array
                    auto* nextVar = (noneZeroElmntMIF) ? 
                        verMod->createProcessLocalVariableMIFNonZero(procView,
                            nextName, verVar->getBitwidth(), verVar->getArrayDims(),
                            verVar->isSigned(), verVar->getInitVals()) :
                        verMod->createProcessLocalVariable(procView,
                            nextName, verVar->getBitwidth(), verVar->getArrayDims(),
                            verVar->isSigned(), verVar->getInitVals());
                    
                    // Add @var <= @var_next in @always_ff body, 
                    // put MIF array suffix if required
                    bool isUnique = verMod->checkProcUniqueVar(procView, verVar);
                    if (zeroElmntMIF || noneZeroElmntMIF || isUnique)
                    {
                        // Declaration of @var and @var_next
                        // No declaration for non-zero elements of MIF array
                        if (!noneZeroElmntMIF) {
                            if (!elabObj->isChannel()) {
                               verMod->convertToProcessLocalVar(verVar, procView);
                            }
                            if (sctCombSignal || sctCombSignClear || 
                                afterResetAcess || !REMOVE_UNUSED_NEXT()) 
                            {
                                verMod->procVarMap[procView].insert(nextVar);
                            }
                        }
                            
                        // MIF array element process or @verVar added first time
                        if (sctCombSignal) {
                            // Swap current and next name 
                            verMod->addProcRegisterNextValPair(
                                procView, nextVar, verVar, mifElemSuffix);
                        } else {
                            if (sctCombSignClear || afterResetAcess || 
                                !REMOVE_UNUSED_NEXT()) {
                                verMod->addProcRegisterNextValPair(
                                    procView, verVar, nextVar, mifElemSuffix);
                            }
                        }
                    }
                    
                    // Add to verVarTraits
                    if (first || isRecordChan) {
                        // Put suffix to @varTraits to use in @var_next = @var
                        SValue vval; 
                        vval = isRecordChan ? 
                               createRecChanField(rcFields[i], regVar) : regVar;
                        globalState->putVerilogTraits(vval,
                            VerilogVarTraits(
                            sctCombSignal ? VerilogVarTraits::COMBSIG : 
                            sctClearSignal ? VerilogVarTraits::CLEARSIG : 
                                             VerilogVarTraits::REGISTER, 
                            accessPlace, true, isNonZeroElmt, false,
                            verVar->getName(), nextVar->getName(), mifElemSuffix));
                        
                        vval = isRecordChan ? 
                               createRecChanField(rcFields[i], regVarZero) : regVarZero;
                        globalState->putVerilogTraits(vval,
                            VerilogVarTraits(
                            sctCombSignal ? VerilogVarTraits::COMBSIG : 
                            sctClearSignal ? VerilogVarTraits::CLEARSIG : 
                                             VerilogVarTraits::REGISTER, 
                            accessPlace, true, !isNonZeroElmt, false,
                            verVar->getName(), nextVar->getName(), mifElemSuffix));
                    }

                    // Register used and defined values
                    verMod->addVarUsedInProc(procView, nextVar, isConst, isChannel);
                    verMod->addVarDefinedInProc(procView, nextVar, isConst, isChannel);
                    verMod->putValueForVerVar(nextVar, regVarZero);
                }
                
                // Register used and defined values
                verMod->addVarUsedInProc(procView, verVar, isConst, isChannel);
                verMod->addVarDefinedInProc(procView, verVar, isConst, isChannel);
                verMod->putValueForVerVar(verVar, regVarZero);
                i++;
            }
            
        } else {
            // Process local variable
            if (!regVar.isVariable()) {
                SCT_INTERNAL_FATAL_NOLOC("Register is not a variable");
                continue;
            }

            // Get name with local record name prefix
            std::string varName = regVar.getVariable().getDecl()->getName().str();
            std::string locRecName = ScState::getLocalRecName(regVar);
            if (!locRecName.empty()) {
                varName = locRecName + "_" + varName;
            }
            
            std::string regName = varName;
            std::string nextName = varName;
            if (VerilogKeywords::check(nextName)) {
                nextName += "_v";
            }
            nextName += "_next";

            //cout << "Process local variable " << regName << " " << nextName << endl;
            //cout << "   zeroElmntMIF " << zeroElmntMIF << " noneZeroElmntMIF " << noneZeroElmntMIF << endl;
            
            // Fill array dimension and get register type
            IndexVec arrayDims;
            QualType regType = getLocalArrayDims(regVar, arrayDims);
            if (isReference(regType)) regType = regType.getNonReferenceType();

            if (!isAnyInteger(regType)) {
                SCT_INTERNAL_FATAL(regVar.getVariable().getDecl()->getBeginLoc(), 
                                   "Incorrect type of local register variable");
                continue;
            }

            // Add register variable to VerilogModule
            auto intTraits = getIntTraits(regType);
            if (!intTraits) {
                SCT_INTERNAL_FATAL(regVar.getVariable().getDecl()->getBeginLoc(), 
                                   "Non-integer type of local register variable");
                continue;
            }
            auto width = intTraits->first;
            auto isSigned = !intTraits->second;
            
            // No variable declaration for non zero elements of MIF array
            auto* verVar = verMod->createProcessLocalVariable(procView,
                                regName, width, arrayDims, isSigned);    
            auto* verNextVar = verMod->createProcessLocalVariable(procView,
                                nextName, width, arrayDims, isSigned);
            verMod->procVarMap[procView].insert(verVar);
            if (afterResetAcess || !REMOVE_UNUSED_NEXT()) {
                verMod->procVarMap[procView].insert(verNextVar);
            }
            
            // Add var <= var_next in @always_ff body
            bool isUnique = verMod->checkProcUniqueVar(procView, verVar);
            //cout << "isUnique " << isUnique << " afterResetAcess " << afterResetAcess << endl;
            if (zeroElmntMIF || noneZeroElmntMIF || isUnique) {
                if (afterResetAcess || !REMOVE_UNUSED_NEXT()) {
                    verMod->addProcRegisterNextValPair(procView, verVar, 
                                                       verNextVar);
                }
            }
            
            // Add to verVarTraits
            globalState->putVerilogTraits(regVar,
                                VerilogVarTraits(VerilogVarTraits::REGISTER, 
                                    accessPlace, false, isNonZeroElmt, false,
                                    verVar->getName(), verNextVar->getName()));
            globalState->putVerilogTraits(regVarZero,
                                VerilogVarTraits(VerilogVarTraits::REGISTER, 
                                    accessPlace, false, !isNonZeroElmt, false, 
                                    verVar->getName(), verNextVar->getName()));
            
            bool isConst = regVar.getType().isConstQualified() || 
                           isPointerToConst(regVar.getType());

            // Register used and defined values
            verMod->addVarUsedInProc(procView, verVar, isConst, 0);
            verMod->addVarDefinedInProc(procView, verVar, isConst, 0);
            verMod->addVarUsedInProc(procView, verNextVar, isConst, 0);
            verMod->addVarDefinedInProc(procView, verNextVar, isConst, 0);
            verMod->putValueForVerVar(verVar, regVarZero);
            verMod->putValueForVerVar(verNextVar, regVarZero);
        }
    }
                                 
    // -------------- Local or member combinational variable
    for (auto combVar : threadCombVars) {
        //cout << "Thread Comb val: " << combVar << endl;
        
        // Ignore record value, record fields are used instead
        if (isUserClass(getDerefType(combVar.getType()))) continue;
        if (isZeroWidthType(getDerefType(combVar.getType()))) continue;
        
        if (!combVar.isVariable()) {
            SCT_TOOL_ASSERT (false, "Combinational variable is not a variable");
            continue;
        }
        
        // Replace value to array zero element 
        // Use global state because only members needs to be processed here
        SValue combVarZero = globalState->getFirstArrayElementForAny(
                                        combVar, ScState::MIF_CROSS_NUM);
        bool isNonZeroElmt = combVar != combVarZero;
        SCT_TOOL_ASSERT (combVarZero, "No zero element found in global state");
        //cout << "combVarZero " << combVarZero << endl;

        // Get access place
        bool inResetAccess = travConst->isInResetAccessed(combVarZero);
        bool afterResetAcess = travConst->isAfterResetAccessed(combVarZero);
        auto accessPlace = VerilogVarTraits::getAccessPlace(inResetAccess, 
                                                            afterResetAcess);
        
        if (auto elabObj = globalState->getElabObject(combVarZero)) {
            //cout << "Global object, elabObj " << elabObj->getDebugString() << endl;
            // Get first array element for non-first element and dereference 
            // pointer to get Verilog variable name
            if (!elabObj) {
                SCT_TOOL_ASSERT (false, "No elaboration object for variable "
                                        "in thread");
                continue;
            }
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }
            
            // Channel cannot be combinational variable
            SCT_TOOL_ASSERT (!elabObj->isChannel(), 
                             "Channel is combinational variable");

            auto verVars = verMod->getVerVariables(*elabObj);
            bool isRecord = elabObj->isRecord();

            // Skip channels of record fields passed by reference
            if (verVars.empty() && combVarZero.isVariable()) {
                if (combVarZero.getVariable().getParent().isScChannel()) continue;
            }
            
            // Record has no variable
            if (verVars.empty() && !isRecord) {
                std::string err = "No combinational variable for elaboration object "
                                  "(generateThreadLocalVariables)";
                if (combVarZero.isVariable()) {
                    SCT_INTERNAL_ERROR(combVarZero.getVariable().getDecl()->
                                       getBeginLoc(), err);
                } else {
                    SCT_INTERNAL_ERROR_NOLOC(err);
                }
            }
            
            bool isReadVar = threadReadVars.count(combVar);
            bool isConst = combVar.getType().isConstQualified() || 
                           isPointerToConst(combVar.getType());
            for (auto* verVar : verVars) {
                isConst = isConst || verVar->isConstant();
            }
            
            // Skip constant variable it is declared as @localparam
            if (isConst) {
                threadReadOnlyVars.insert(combVar);
                continue;
            }
            
            // Add thread variable declaration    
            if (!verVars.empty()) {
                if (zeroElmntMIF || noneZeroElmntMIF ||
                    verMod->checkProcUniqueVar(procView, verVars[0])) {
                    // Do not generate variable declarations for non-zero elements of MIF array
                    if (!noneZeroElmntMIF) {
                        for (auto* verVar : verVars) {
                            verMod->convertToProcessLocalVar(verVar, procView);
                        }
                    }
                }
            }

            // Add verVarTraits
            if (!verVars.empty()) {
                globalState->putVerilogTraits(combVar, VerilogVarTraits(
                                VerilogVarTraits::COMB, accessPlace, true, 
                                isNonZeroElmt, false, verVars[0]->getName()));
                globalState->putVerilogTraits(combVarZero, VerilogVarTraits(
                                VerilogVarTraits::COMB, accessPlace, true, 
                                !isNonZeroElmt, false, verVars[0]->getName()));
            }

            
            // Register used/defined values
            for (auto* verVar : verVars) {
                if (isReadVar) {
                    verMod->addVarUsedInProc(procView, verVar, isConst, 0);
                }
                verMod->addVarDefinedInProc(procView, verVar, isConst, 0);
                verMod->putValueForVerVar(verVar, combVarZero);
            }

        } else {
            // Local variable, generated in process body
            globalState->putVerilogTraits(combVar, VerilogVarTraits(
                                          VerilogVarTraits::COMB, accessPlace, 
                                          false, isNonZeroElmt, false));
            globalState->putVerilogTraits(combVarZero, VerilogVarTraits(
                                          VerilogVarTraits::COMB, accessPlace, 
                                          false, !isNonZeroElmt, false));
        }
    }                                 

    // ---------- Read only including constants
    for (auto roVar : threadReadOnlyVars) {
        //cout << "Thread Read-only val: " << roVar << endl;

        // Ignore record value, record fields are used instead
        if (isUserClass(getDerefType(roVar.getType()))) continue;
        if (isZeroWidthType(getDerefType(roVar.getType()))) continue;
        
        // Replace value to array first element 
        // Use global state because only members needs to be processed here
        SValue roVarZero = globalState->getFirstArrayElementForAny(
                                    roVar,  ScState::MIF_CROSS_NUM);
        bool isNonZeroElmt = roVar != roVarZero;
        SCT_TOOL_ASSERT (roVarZero, "No zero element found in global state");
        
        // Get access place
        bool inResetAccess = travConst->isInResetAccessed(roVarZero);
        bool afterResetAcess = travConst->isAfterResetAccessed(roVarZero);
        auto accessPlace = VerilogVarTraits::getAccessPlace(inResetAccess, 
                                                            afterResetAcess);

        if (auto elabObj = globalState->getElabObject(roVarZero)) {
            // Get first array element for non-first element and dereference 
            // pointer to get Verilog variable name
            if (!elabObj) {
                SCT_TOOL_ASSERT (false, "No elaboration object for variable "
                                        "in thread");
                continue;
            }
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }

            if (elabObj->isChannel()) {
                // For channels and channel arrays process analyzer wants to
                // see ScChannel SValue of first (or singular) array element
                // instead of array/pointer SVariable
                while (!roVar.isScChannel() && !roVar.isUnknown()) {
                    roVar = globalState->getValue(roVar);
                }
                SCT_TOOL_ASSERT(roVar.isScChannel(), "No channel found");
                while (!roVarZero.isScChannel() && !roVarZero.isUnknown()) {
                    roVarZero = globalState->getValue(roVarZero);
                }
                SCT_TOOL_ASSERT(roVarZero.isScChannel(), "No channel found");
            }

            // Module data members
            auto verVars = verMod->getVerVariables(*elabObj);
            
            // Skip channels of record fields passed by reference
            if (verVars.empty() && roVarZero.isVariable()) {
                if (roVarZero.getVariable().getParent().isScChannel()) continue;
            }

            // Create constant in module, required for module where from
            // called MIF function which access the constant
            if (verVars.empty() && roVarZero.isVariable()) {
                // Name uniqueness provided in @createDataVariable()
                std::string name = roVarZero.getVariable().getDecl()->
                                   getNameAsString();
                
                QualType type = roVarZero.getType();
                size_t bitwidth = 64;
                bool isSigned = 0; 
                if (auto traits = getIntTraits(type)) {
                    bitwidth = traits->first;
                    isSigned = traits->second;
                }
                
                IndexVec arrayDims;
                APSIntVec initVals;
                SValue val = globalState->getValue(roVarZero);
                if (val.isInteger()) {
                    initVals.emplace_back(val.getInteger());
                } else {
                    std::string err = "No value for read-only variable"
                                      "(generateThreadLocalVariables)";
                    SCT_INTERNAL_ERROR(roVarZero.getVariable().getDecl()->
                                       getBeginLoc(), err);
                }
                
                verMod->createDataVariable(*elabObj, name, bitwidth, arrayDims,
                                           isSigned, initVals);
                verVars = verMod->getVerVariables(*elabObj);
            }
            
            bool isNullPtr = false;
            bool isDanglPtr = false;

            if (isPointer(roVar.getType())) {
                SValue rval = globalState->getValue(roVar);
                isNullPtr = rval.isInteger() && rval.getInteger().isZero();
                isDanglPtr = rval.isUnknown();
            }
            
            bool isRecord = elabObj->isRecord();
            
            // Constant dangling/null pointer and record have no variable
            if (verVars.empty() && !isNullPtr && !isDanglPtr && !isRecord) {
                std::string err = "No read-only variable for elaboration object "
                                  "(generateThreadLocalVariables)";
                if (roVarZero.isVariable()) {
                    SCT_INTERNAL_ERROR(roVarZero.getVariable().getDecl()->
                                       getBeginLoc(), err);
                } else {
                    SCT_INTERNAL_ERROR_NOLOC(err);
                }
            }
            
            bool isChannel = roVar.isScChannel();
            bool isConst = !isChannel && (roVar.getType().isConstQualified()||
                                          isPointerToConst(roVar.getType()));
            bool first = true;
            for (auto* verVar : verVars) {
                // Add to verVarTraits
                if (first) {
                    globalState->putVerilogTraits(roVar,
                        VerilogVarTraits(VerilogVarTraits::READONLY, accessPlace,
                                         true, isNonZeroElmt, verVar->isConstant(),
                                         verVar->getName()));
                    globalState->putVerilogTraits(roVarZero,
                        VerilogVarTraits(VerilogVarTraits::READONLY, accessPlace, 
                                         true, !isNonZeroElmt, verVar->isConstant(),
                                         verVar->getName()));
                    first = false;
                }
                
                // Register used values
                verMod->addVarUsedInProc(procView, verVar, isConst, isChannel);
                verMod->putValueForVerVar(verVar, roVarZero);
            }
            
        } else {
            // Process local constants
            //cout << "Local constant defined in reset " << roVar << endl;
            if (travConst->getResetDefConsts().count(roVar)) {
                // Local constant defined in reset section, 
                // it needs to be declared in module scope
                if (!roVar.isVariable()) {
                    SCT_INTERNAL_FATAL_NOLOC("Process local constant is not a variable");
                    continue;
                }

                // Get name with local record name prefix
                std::string varName = roVar.getVariable().getDecl()->getName().str();
                std::string locRecName = ScState::getLocalRecName(roVar);
                if (!locRecName.empty()) {
                    varName = locRecName + "_" + varName;
                }
                
                IndexVec arrayDims;
                QualType roType = getLocalArrayDims(roVar, arrayDims);
                if (isReference(roType)) roType = roType.getNonReferenceType();

                if (!isAnyInteger(roType)) {
                    SCT_INTERNAL_FATAL(roVar.getVariable().getDecl()->getBeginLoc(), 
                                       "Incorrect type of local constant variable");
                    continue;
                }

                // Add process local variable to VerilogModule
                auto intTraits = getIntTraits(roType);
                if (!intTraits) {
                    SCT_INTERNAL_FATAL(roVar.getVariable().getDecl()->getBeginLoc(), 
                                       "Non-integer type of local constant variable");
                    continue;
                }
                auto width = intTraits->first;
                auto isSigned = !intTraits->second;

                // No variable declaration for non zero elements of MIF array
                auto* verVar = verMod->createProcessLocalVariable(procView,
                                    varName, width, arrayDims, isSigned);
                verMod->procConstMap[procView].push_back(verVar);
                
                // Variable used values
                verMod->addVarUsedInProc(procView, verVar, 1, 0);
                verMod->putValueForVerVar(verVar, roVarZero);

                // Add to verVarTraits, this constant not declared in always blocks
                globalState->putVerilogTraits(roVar, VerilogVarTraits(
                                     VerilogVarTraits::READONLY_CDR, accessPlace,
                                     true, isNonZeroElmt, false, verVar->getName()));
                globalState->putVerilogTraits(roVarZero, VerilogVarTraits(
                                     VerilogVarTraits::READONLY_CDR, accessPlace,
                                     true, !isNonZeroElmt, false, verVar->getName()));
                
            } else {
                // Other local constant
                globalState->putVerilogTraits(roVar, VerilogVarTraits(
                                    VerilogVarTraits::READONLY, accessPlace, 
                                    false, isNonZeroElmt, false));
                globalState->putVerilogTraits(roVarZero, VerilogVarTraits(
                                    VerilogVarTraits::READONLY, accessPlace, 
                                    false, !isNonZeroElmt, false));
            }
        }
    }
    
 
    
    // Add reset signal to used variables, required if reset is @sc_signal
    for (const auto& reset : procView.resets()) {
        ObjectView resetObj = reset.sourceObj;
        auto arrElm = resetObj.getAsArrayElementWithIndicies();
        resetObj = arrElm.obj;

        auto i = verMod->channelVarMap.find(resetObj);
        if (i == verMod->channelVarMap.end()) {
            SCT_INTERNAL_ERROR(procView.getLocation().second->getBeginLoc(), 
                               "No variable for reset object");
        }
        
        for (auto* var : i->second) {
            verMod->addVarUsedInProc(procView, var, 0, 1);
        }
    }
    
    // Add sensitivity signal to used variables, required for clock in MIF
    for (auto event : procView.staticSensitivity()) {
        ObjectView eventObj = event.sourceObj;
        auto arrElm = eventObj.getAsArrayElementWithIndicies();
        eventObj = arrElm.obj;

        auto i = verMod->channelVarMap.find(eventObj);
        if (i == verMod->channelVarMap.end()) {
            SCT_INTERNAL_ERROR(procView.getLocation().second->getBeginLoc(), 
                               "No variable for sensitivity list object");
        }
        
        for (auto* var : i->second) {
            verMod->addVarUsedInProc(procView, var, 0, 1);
        }
    }
    
//    cout << "----- Used variables: " << procView.procName << endl;
//    for (const auto& entry : verMod->procUseVars[procView]) {
//        cout << entry.first->getName() << endl;
//    }
//    cout << "----- Defined variables: " << procView.procName << endl;
//    for (const auto& entry : verMod->procDefVars[procView]) {
//        cout << entry.first->getName() << endl;
//    }
    
    DEBUG_WITH_TYPE(DebugOptions::doThreadBuild, globalState->print());
}


} // end namespace sc

