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

#include "sc_tool/cfg/ScTraverseConst.h"
#include "sc_tool/cfg/ScStmtInfo.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/cthread/ScCFGAnalysis.h"
#include "sc_tool/cthread/ScSingleStateThread.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/ScCommandLine.h"
#include "sc_tool/utils/BitUtils.h"
#include "sc_tool/utils/VerilogKeywords.h"
#include <sstream>

using namespace llvm;
using namespace clang;
using namespace sc_elab;

namespace sc
{
    
ThreadBuilder::ThreadBuilder(const clang::ASTContext &astCtx,
                             sc_elab::ElabDatabase &elabDB,
                             const sc_elab::ProcessView procView,
                             std::shared_ptr<ScState> globalState,
                             const SValue &modval,
                             const SValue &dynmodval
                             ) :
      sm(astCtx.getSourceManager()),
      astCtx(astCtx),
      cfgFab(*CfgFabric::getFabric(astCtx)),
      entryFuncDecl(procView.getLocation().second),
      findWaitVisitor(),
      elabDB(elabDB),
      globalState(globalState),
      modSval(modval),
      dynModSval(dynmodval),
      threadStates(procView.getLocation().second, astCtx),
      procView(procView)
{}

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

sc_elab::VerilogProcCode ThreadBuilder::run(bool constPropOnly)
{
    using std::cout; using std::endl;
    // Run CPA
    runConstProp();

    // Pass over all states, if first statement after wait() is infinite loop
    // and it is the same for all wait() the thread is single thread
    // The single state process must have reset
    isSingleState = isSingleStateThread(threadStates) &&
                    !procView.resets().empty() &&
                    !globalConstProp->isBreakInRemovedLoop();

    // Fill verVarTrais and put defined non-channel variables to process 
    // variable storage to be generated before the process
    if (!singleBlockCThreads) {
        generateThreadLocalVariables();
    }
    
    //globalState->print();

    if (!constPropOnly) {
        // Generate thread case and process body
        vGen = std::make_unique<ScThreadVerilogGen>(procView, astCtx,
            threadStates, *globalState, findWaitVisitor, modSval,
            dynModSval, isSingleState, stateRegNames, waitNRegNames);

        runVerilogGeneration();
        
        auto parentModView = procView.getParentModule();
        auto verMod  = elabDB.getVerilogModule(parentModView);
        
        // Add constants not replaced with integer value, 
        // used to determine required variables
        for (const SValue& val : vGen->getNotReplacedVars()) {
            verMod->addNotReplacedVar(val);
        }
        
        return vGen->getVerilogCode();
    }

    return VerilogProcCode("");
}

void ThreadBuilder::runConstProp()
{
    using namespace std;
    if (DebugOptions::isEnabled(DebugComponent::doConstStmt) ||
        DebugOptions::isEnabled(DebugComponent::doConstTerm) ||
        DebugOptions::isEnabled(DebugComponent::doConstLoop) ||
        DebugOptions::isEnabled(DebugComponent::doConstCfg)) {
        std::cout << endl << "--------------- THREAD GLOBAL CPA : " 
                  << entryFuncDecl->getNameAsString() << endl;
    }
    
    // Traverse context stack, used to run TraverseProc
    traverseContextMap[RESET_ID] = vGen->getInitialTraverseProcState();
    // State to fill by CPA
    std::shared_ptr<ScState> globalConstPropState(globalState->clone());

    // Run global CPA, used to provide initial state for local CPA
    globalConstProp = std::make_unique<ScTraverseConst>(astCtx,
                            globalConstPropState, modSval, 
                            globalState, &elabDB, &threadStates, 
                            &findWaitVisitor, false);
    
    bool hasReset = !procView.resets().empty();
    globalConstProp->setHasReset(hasReset);
    
    globalConstProp->run(entryFuncDecl);

    // Check at least one @wait achieved
    if (globalConstProp->getWaitStates().empty() && !constPropOnly) {
        ScDiag::reportScDiag(procView.getLocation().second->getBeginLoc(),
                             ScDiag::SC_FATAL_THREAD_NO_STATE);
        SCT_INTERNAL_FATAL_NOLOC ("No states found for thread");
    }
    
    // Fill UseDef for all states
    for (const auto& entry : globalConstProp->getWaitStates()) {
        analyzeUseDefResults(&entry.second);
    }
    
    // Report read not initialized warning
    auto notInitReport = [&](const InsertionOrderSet<SValue>& values) {
        for (auto sval : values) {
            // Skip constants, channels and pointers to channel
            QualType type = sval.getType();
            if (type.isConstQualified() || isConstReference(type) || 
                isPointerToConst(type) ||
                isScChannel(type) || isScVector(type) || 
                isScChannelArray(type) || 
                isScToolCombSignal(type)) continue;

            // Skip null pointer
            if (isPointer(type)) {
                SValue rval = globalState->getValue(sval);
                if (rval.isInteger() && rval.getInteger().isNullValue()) {
                    continue;
                }
            }

            std::string varName = sval.isVariable() ? 
                                  sval.getVariable().asString(false) : "---";
            // Do not check duplicates
            ScDiag::reportScDiag(entryFuncDecl->getBeginLoc(), 
                                 ScDiag::CPP_READ_NOTDEF_VAR, false) << varName;
        }
    };
    
    // Report warning for read not initialized in the same cycle where declared
    for (const auto& entry : globalConstProp->getWaitStates()) {
        notInitReport(entry.second.getReadNotInitValues());
    }
    // Report warning for read and never initialized in the following cycles 
    notInitReport(threadReadNotDefVars);
    threadReadNotDefVars.clear();
}

void ThreadBuilder::analyzeUseDefResults(const ScState* finalState)
{
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
    for (const auto& val : finalState->getReadValues()) {
        if (val.isVariable() || val.isObject()) {
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
                if (rval.isInteger() && rval.getInteger().isNullValue())
                    continue;
            }

            threadReadVars.insert(val);
        }
    }
    
    // Process read-not-defined
    for (const auto& sval : finalState->getReadNotDefinedValues()) {
        DEBUG_WITH_TYPE(DebugOptions::doThreadBuild,
                        outs() << "READ_NOT_DEF: " << sval<< "\n";);

        if (sval.isVariable() || sval.isObject()) {
            // Check @sc_comb_sig read before defined, report error if so
            QualType qtype = sval.getType();
            if (isScToolCombSignal(qtype)) {
                // @sc_comb_sig has second argument @CLEAR
                bool sctCombSignClear = false;
                if (auto clearArg = getTemplateArgAsInt(qtype, 1)) {
                    sctCombSignClear = !clearArg->isNullValue();
                }

                // Report fatal error as it generates incorrect code
                if (sctCombSignClear) {
                    combSignClearRnDs.insert(sval);
                }
            }
            
            bool isConsRef = sval.isConstReference();
            bool isConsVal = !isConsRef && ScState::isConstVarOrLocRec(sval);

            if (isConsVal || isConsRef) {
                // Member constant
                bool isMember = globalState->getElabObject(sval).hasValue();

                // Check if local constant has integer value to replace it or
                // defined in reset means its value available in all states 
                // Such local constant considered as read only, no register
                bool readOnlyConst = false;
                if (!isMember) {
                    const SValue& rval = finalState->getValue(sval);
                    readOnlyConst = replaceConstByValue && rval.isInteger() || 
                                    isConsVal && globalConstProp->
                                    getResetDefConsts().count(sval);
                }
                        
                if (isMember || readOnlyConst) {
                    // Member and local replaced constant becomes read only
                    if (!threadRegVars.count(sval) && 
                        !threadReadOnlyVars.count(sval)) {
                        threadReadOnlyVars.insert(sval);
                    }
                } else {
                    // Local not replaced constant becomes registers
                    if (!threadRegVars.count(sval)) {
                        threadRegVars.insert(sval); 
                    }
                    if (threadReadOnlyVars.count(sval)) {
                        threadReadOnlyVars.erase(sval);
                    }
                }
                // TODO: Uncomment me, #247
//                assert (!threadRegVars.count(sval));
//                if (!threadReadOnlyVars.count(sval)) {
//                    threadReadOnlyVars.insert(sval);
//                }

            } else 
            if (isScChannel(sval.getType()) || isScVector(sval.getType()) ||
                isScChannelArray(sval.getType()) || 
                isScToolCombSignal(sval.getType())) 
            {
                // RnD channel becomes read only if it is not defined later
                if (!threadRegVars.count(sval)) {
                    threadReadOnlyVars.insert(sval);
                }
            } else {
                // Put variable into ReadNotDefined if meet it first time,
                // used to report error only
                if (!threadRegVars.count(sval) && !threadCombVars.count(sval)) {
                    threadReadNotDefVars.insert(sval);
                }
                // RnD variable becomes register
                if (threadCombVars.count(sval)) {
                    threadCombVars.erase(sval);
                }
                threadRegVars.insert(sval);
            }
        } else {
            llvm::outs() << "val " << sval << "\n";
            SCT_TOOL_ASSERT (false, "Unexpected value type");
        }
    }

    // Processing define values
    for (const auto& sval : finalState->getDefArrayValues()) {
        DEBUG_WITH_TYPE(DebugOptions::doThreadBuild,
                        outs() << "ARRAY DEFINED: " << sval<< "\n";);

        if (combSignClearRnDs.count(sval)) {
            ScDiag::reportScDiag(
                procView.getLocation().second->getBeginLoc(),
                ScDiag::SYNTH_COMB_SIG_READNOTDEF);
        }
         
        if (isScChannel(sval.getType()) || isScVector(sval.getType()) ||
            isScChannelArray(sval.getType()) || 
            isScToolCombSignal(sval.getType())) 
        {
            // Defined channel becomes register
            if (threadReadOnlyVars.count(sval)) {
                threadReadOnlyVars.erase(sval);
            }
            threadRegVars.insert(sval);
            
        } else 
        if (sval.isVariable() || sval.isObject()) {
            
            bool isConsRef = sval.isConstReference();
            bool isConsVal = !isConsRef && ScState::isConstVarOrLocRec(sval);

            // Defined variable becomes combinational if it is not register
            if (!isConsVal && !isConsRef) {
                if (!threadRegVars.count(sval)) {
                    threadCombVars.insert(sval);
                }
                if (threadReadNotDefVars.count(sval)) {
                    threadReadNotDefVars.erase(sval);
                }
            }
            
            // Check member constant variables not defined
            if (sval.isVariable() && sval.getType().isConstQualified()) {
                const ValueDecl* valDecl = sval.getVariable().getDecl();
                const DeclContext* declContext = valDecl->getDeclContext();
                bool isLocalVar = isa<FunctionDecl>(declContext);

                if (!isLocalVar) {
                    ScDiag::reportScDiag(
                        procView.getLocation().second->getBeginLoc(),
                        ScDiag::SYNTH_CONST_VAR_MODIFIED) << sval.asString(false);
                }
            }
            
        } else {
            llvm::outs() << "val " << sval << "\n";
            SCT_TOOL_ASSERT (false, "Unexpected value type");
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

void ThreadBuilder::runVerilogGeneration()
{
    using namespace std;
    // Reused for code generation
    visitedStates.clear();
    scheduledStates.clear();

    // Generate reset section
    auto reachableWaits = generateVerilogForState(RESET_ID);
    scheduledStates.insert(reachableWaits.begin(), reachableWaits.end());

    while (!scheduledStates.empty()) {

        WaitID waitID = *scheduledStates.begin();
        scheduledStates.erase(scheduledStates.begin());

        if (visitedStates.count(waitID))
            continue;

        visitedStates.insert(waitID);

        auto reachableWaits = generateVerilogForState(waitID);
        scheduledStates.insert(reachableWaits.begin(), reachableWaits.end());
        
        if (isSingleState) break;
    }
}

// Fill @traverseContextMap and return reachable wait IDs
std::set<WaitID> ThreadBuilder::generateVerilogForState(WaitID stateID)
{
    std::set<WaitID> reachableWaits;
    
    if (traverseContextMap.count(stateID)) {
        auto startTraverseContext = traverseContextMap.at(stateID);
        auto waitProcContexts = vGen->generateCodePath(startTraverseContext, 
                                stateID, *(globalConstProp.get()));

        for (auto &waitCtx : waitProcContexts) {
            reachableWaits.insert(waitCtx.first);
            traverseContextMap[waitCtx.first] = waitCtx.second;
        }
    }
    return reachableWaits;
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
    bool zeroElmntMIF = globalConstProp->isZeroElmtMIF();
    bool noneZeroElmntMIF = globalConstProp->isNonZeroElmtMIF();
    std::string mifElemSuffix = (zeroElmntMIF || noneZeroElmntMIF) ?
                                 globalConstProp->getMifElmtSuffix() : "";
    //cout << "Thread is zeroElmntMIF " << zeroElmntMIF << " noneZeroElmntMIF " 
    //     << noneZeroElmntMIF << " nonZeroSuffix " << mifElemSuffix << endl;
    
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
        if (isUserDefinedClass(regVar.getType())) continue;
        
        // Get access place, access after reset includes access in SVA
        bool inResetAccess = globalConstProp->isInResetAccessed(regVar);
        bool afterResetAcess = globalConstProp->isAfterResetAccessed(regVar) ||
                               globalConstProp->isSvaProcAccessed(regVar);
        auto accessPlace = VerilogVarTraits::getAccessPlace(inResetAccess, 
                                                            afterResetAcess);

        // Replace value to array first element 
        // Use global state because only members needs to be processed here
        SValue regVarZero = globalState->getFirstArrayElementForAny(
                                        regVar, ScState::MIF_CROSS_NUM);
        bool isNonZeroElmt = regVar != regVarZero;
        SCT_TOOL_ASSERT (regVarZero, "No zero element found in global state");
        //cout << "   regVarZero " << regVarZero << " isNonZeroElemnt " << isNonZeroElmt << endl;
        
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
            
            if (elabObj->isChannel()) {
                QualType qtype = elabObj->getType();
                sctCombSignal = isScToolCombSignal(qtype);

                if (sctCombSignal) {
                    // @sc_comb_sig has second argument @CLEAR
                    if (auto clearArg = getTemplateArgAsInt(qtype, 1)) {
                        sctCombSignClear = !clearArg->isNullValue();
                    }
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
                isNullPtr = rval.isInteger() && rval.getInteger().isNullValue();
                isDanglPtr = rval.isUnknown();
            }
            
            // Module data members
            auto verVars = verMod->getVerVariables(*elabObj);
            bool isRecord = elabObj->isRecord();
            
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
            
            bool first = true;
            // Loop required for record
            for (auto* verVar : verVars) {
                // Add var <= var_next in @always_ff body
                if (sctCombSignal && sctCombSignClear) {
                    // Add to verVarTraits
                    if (first) {
                        // Swap current and next name (next name is empty)
                        // Put suffix to @varTraits to use in @var_next = @var
                        globalState->putVerilogTraits(regVar,
                            VerilogVarTraits(VerilogVarTraits::COMBSIGCLEAR, 
                            accessPlace, true, isNonZeroElmt,
                            verVar->getName(), llvm::Optional<std::string>(""),
                            mifElemSuffix));
                        globalState->putVerilogTraits(regVarZero,
                            VerilogVarTraits(VerilogVarTraits::COMBSIGCLEAR, 
                            accessPlace, true, !isNonZeroElmt,
                            verVar->getName(), llvm::Optional<std::string>(""),
                            mifElemSuffix));
                        first = false;
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
                    if (zeroElmntMIF || noneZeroElmntMIF ||
                        verMod->checkProcUniqueVar(procView, verVar))
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
                    if (first) {
                        // Put suffix to @varTraits to use in @var_next = @var
                        globalState->putVerilogTraits(regVar,
                            VerilogVarTraits(sctCombSignal ?
                            VerilogVarTraits::COMBSIG : VerilogVarTraits::REGISTER, 
                            accessPlace, true, isNonZeroElmt, verVar->getName(), 
                            nextVar->getName(), mifElemSuffix));
                        globalState->putVerilogTraits(regVarZero,
                            VerilogVarTraits(sctCombSignal ?
                            VerilogVarTraits::COMBSIG : VerilogVarTraits::REGISTER, 
                            accessPlace, true, !isNonZeroElmt, verVar->getName(), 
                            nextVar->getName(), mifElemSuffix));
                        first = false;
                    }

                    // Register used and defined values
                    verMod->addVarUsedInProc(procView, nextVar, isConst, isChannel);
                    verMod->addVarDefinedInProc(procView, nextVar, isConst, isChannel);
                    verMod->putValueForVerVar(nextVar, regVarZero);
                }
                
                // Register used and defined values
                verMod->addVarUsedInProc(procView, verVar, isConst, isChannel);
                    verMod->addVarDefinedInProc(procView, verVar, isConst, 
                                                isChannel);
                verMod->putValueForVerVar(verVar, regVarZero);
            }
            
        } else {
            // Process local variable
            if (!regVar.isVariable()) {
                SCT_INTERNAL_FATAL_NOLOC("Register is not a variable");
                continue;
            }

            // Get name with local record name prefix
            std::string varName = regVar.getVariable().getDecl()->getName();
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
            if (zeroElmntMIF || noneZeroElmntMIF ||
                verMod->checkProcUniqueVar(procView, verVar)) 
            {
                if (afterResetAcess || !REMOVE_UNUSED_NEXT()) {
                    verMod->addProcRegisterNextValPair(procView, verVar, 
                                                       verNextVar);
                }
            }
            
            // Add to verVarTraits
            globalState->putVerilogTraits(regVar,
                                VerilogVarTraits(VerilogVarTraits::REGISTER, 
                                    accessPlace, false, isNonZeroElmt,
                                    verVar->getName(), verNextVar->getName()));
            globalState->putVerilogTraits(regVarZero,
                                VerilogVarTraits(VerilogVarTraits::REGISTER, 
                                    accessPlace, false, !isNonZeroElmt,
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
        if (isUserDefinedClass(combVar.getType())) continue;
        
        if (!combVar.isVariable()) {
            SCT_TOOL_ASSERT (false, "Combinational variable is not a variable");
            continue;
        }
        
        // Get access place
        bool inResetAccess = globalConstProp->isInResetAccessed(combVar);
        bool afterResetAcess = globalConstProp->isAfterResetAccessed(combVar);
        auto accessPlace = VerilogVarTraits::getAccessPlace(inResetAccess, 
                                                            afterResetAcess);
                        
        // Replace value to array zero element 
        // Use global state because only members needs to be processed here
        SValue combVarZero = globalState->getFirstArrayElementForAny(
                                        combVar, ScState::MIF_CROSS_NUM);
        bool isNonZeroElmt = combVar != combVarZero;
        SCT_TOOL_ASSERT (combVarZero, "No zero element found in global state");
        //cout << "combVarZero " << combVarZero << endl;

        if (auto elabObj = globalState->getElabObject(combVarZero)) {
            //cout << "Global object, elabObj " << elabObj.getValue().getDebugString() << endl;
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
                                isNonZeroElmt, verVars[0]->getName()));
                globalState->putVerilogTraits(combVarZero, VerilogVarTraits(
                                VerilogVarTraits::COMB, accessPlace, true, 
                                !isNonZeroElmt, verVars[0]->getName()));
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
                                          VerilogVarTraits::COMB,
                                          accessPlace, false, isNonZeroElmt));
            globalState->putVerilogTraits(combVarZero, VerilogVarTraits(
                                          VerilogVarTraits::COMB,
                                          accessPlace, false, !isNonZeroElmt));
        }
    }                                 

    // ---------- Read only including constants
    for (auto roVar : threadReadOnlyVars) {
        //cout << "Thread Read-only val: " << roVar << endl;

        // Ignore record value, record fields are used instead
        if (isUserDefinedClass(roVar.getType())) continue;
        
        // Get access place
        bool inResetAccess = globalConstProp->isInResetAccessed(roVar);
        bool afterResetAcess = globalConstProp->isAfterResetAccessed(roVar);
        auto accessPlace = VerilogVarTraits::getAccessPlace(inResetAccess, 
                                                            afterResetAcess);
        
        // Replace value to array first element 
        // Use global state because only members needs to be processed here
        SValue roVarZero = globalState->getFirstArrayElementForAny(
                                    roVar,  ScState::MIF_CROSS_NUM);
        bool isNonZeroElmt = roVar != roVarZero;
        SCT_TOOL_ASSERT (roVarZero, "No zero element found in global state");
        
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
                isNullPtr = rval.isInteger() && rval.getInteger().isNullValue();
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
            for (auto *verVar : verVars) {
                // Add to verVarTraits
                if (first) {
                    globalState->putVerilogTraits(roVar,
                        VerilogVarTraits(VerilogVarTraits::READONLY, accessPlace,
                                         true, isNonZeroElmt, verVar->getName()));
                    globalState->putVerilogTraits(roVarZero,
                        VerilogVarTraits(VerilogVarTraits::READONLY, accessPlace, 
                                         true, !isNonZeroElmt, verVar->getName()));
                    first = false;
                }
                
                // Register used values
                verMod->addVarUsedInProc(procView, verVar, isConst, isChannel);
                verMod->putValueForVerVar(verVar, roVarZero);
            }
            
        } else {
            // Process local constants
            //cout << "Local constant defined in reset " << roVar << endl;
            // TODO: comment me, #247
            if (globalConstProp->getResetDefConsts().count(roVar)) {
                // Local constant defined in reset section, 
                // it needs to be declared in module scope
                if (!roVar.isVariable()) {
                    SCT_INTERNAL_FATAL_NOLOC("Process local constant is not a variable");
                    continue;
                }

                // Get name with local record name prefix
                std::string varName = roVar.getVariable().getDecl()->getName();
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

                // TODO: uncomment me, #247
//                bool definedInReset = globalConstProp->getResetDefConsts().count(roVar);
//                auto varKind = definedInReset ? VerilogVarTraits::READONLY_CDR : 
//                                                VerilogVarTraits::READONLY;
                
                // Add to verVarTraits, this constant not declared in always blocks
                globalState->putVerilogTraits(roVar, VerilogVarTraits(
                                     VerilogVarTraits::READONLY_CDR, accessPlace,
                                     true, isNonZeroElmt, verVar->getName()));
                globalState->putVerilogTraits(roVarZero, VerilogVarTraits(
                                     VerilogVarTraits::READONLY_CDR, accessPlace,
                                     true, !isNonZeroElmt, verVar->getName()));
                
            } else {
                // TODO: comment me, #247
                // Other local constant
                globalState->putVerilogTraits(roVar, 
                            VerilogVarTraits(VerilogVarTraits::READONLY, 
                                             accessPlace, false, isNonZeroElmt));
                globalState->putVerilogTraits(roVarZero, 
                            VerilogVarTraits(VerilogVarTraits::READONLY, 
                                             accessPlace, false, !isNonZeroElmt));
            }
        }
    }
    
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

