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

#include "ScProcAnalyzer.h"

#include "sc_tool/cfg/ScTraverseProc.h"
#include "sc_tool/cfg/ScTraverseConst.h"
#include "sc_tool/cfg/ScStmtInfo.h"
#include "sc_tool/cthread/ScCThreadStates.h"
#include "utils/CheckCppInheritance.h"
#include <sc_tool/cthread/ScThreadBuilder.h>
#include <sc_tool/elab/ScObjectView.h>
#include <sc_tool/utils/NameGenerator.h>
#include <sc_tool/utils/CfgFabric.h>
#include <sc_tool/utils/DebugOptions.h>
#include <sc_tool/utils/StringFormat.h>
#include <sc_tool/utils/CppTypeTraits.h>
#include <sc_tool/utils/InsertionOrderSet.h>
#include <sc_tool/ScCommandLine.h>
#include <clang/Analysis/CFG.h>
#include <iostream>
#include <chrono>
#include <sstream>

namespace sc {
    
using namespace clang;
using namespace std;

sc_elab::VerilogProcCode ScProcAnalyzer::analyzeMethodProcess (
                    const SValue& modval,
                    const SValue& dynmodval, 
                    sc_elab::ProcessView& procView)
{
    using namespace sc_elab;

    if (DebugOptions::isEnabled(DebugComponent::doModuleBuilder)) {
        std::cout << "  Analyze method process: "
        << procView.getLocation().first.getType()->getAsCXXRecordDecl()->getNameAsString()
        << " :: " << procView.getLocation().second->getNameAsString() << std::endl;
    }
    
    clang::CXXMethodDecl* methodDecl = procView.getLocation().second;
    bool emptySensitivity = procView.staticSensitivity().empty();
    
    // Module in elaboration DB
    auto parentModView = procView.getParentModule();
    auto verMod  = elabDB.getVerilogModule(parentModView);

    //cout << "--------- proc #" << procView.getID() << endl;
    //clang::CXXMethodDecl* methodDecl = procView.getLocation().second;
    //cout << (methodDecl ? methodDecl->getBeginLoc().printToString(sm) << "" << endl;

    // Print state before process analysis
    if (DebugOptions::isEnabled(DebugComponent::doGenState)) {
        std::cout << endl << "=================== STATE ===================" << endl;
        globalState->print();
    
        std::cout << endl << "=============================================" << endl;

        std::cout << endl << "=============================================" << endl;
        std::cout << " Build CFG for PROCESS : " << methodDecl->getNameAsString() << endl;
        std::cout << " Is sensitivity empty? : " << emptySensitivity << endl;
    }

    if (DebugOptions::isEnabled(DebugComponent::doConstStmt) ||
        DebugOptions::isEnabled(DebugComponent::doConstTerm) ||
        DebugOptions::isEnabled(DebugComponent::doConstLoop) ||
        DebugOptions::isEnabled(DebugComponent::doConstCfg)) {
        std::cout << endl << "--------------- METHOD CPA : " 
                  << methodDecl->getNameAsString() << endl;
    }
    // Clone module state for constant propagation
    if (DebugOptions::isEnabled(DebugComponent::doConstProfile)) {
        cout << "METHOD " << methodDecl->getNameAsString() << " "; 
        globalState->printSize();
    }
    
    // Preliminary CPA
    unordered_set<SValue> defVals;
    DebugOptions::suspend();
    auto preState = shared_ptr<ScState>(globalState->clone());
    ScTraverseConst preConst(astCtx, preState, modval, 
                             globalState, &elabDB, nullptr, nullptr, true, true);
    preConst.run(verMod, methodDecl);
    
    for (const auto& sval : preConst.getFinalState()->getDefArrayValues()) {
        defVals.insert(sval);
    }

    // Remove defined member variables from state
    globalState->removeDefinedValues(defVals);
    DebugOptions::resume();
    
    // Main CPA
    if (DebugOptions::isEnabled(DebugComponent::doModuleBuilder)) {
        cout << "\n=========================  MAIN CPA =============================\n";
    }
    auto start = chrono::system_clock::now();
    auto constState = shared_ptr<ScState>(globalState->clone());
    ScTraverseConst travConst(astCtx, constState, modval, 
                              globalState, &elabDB, nullptr, nullptr, true, true);
    travConst.run(verMod, methodDecl);
    
    ScState* finalState = travConst.getFinalState();
    
    if (DebugOptions::isEnabled(DebugComponent::doConstProfile)) {
        auto end = chrono::system_clock::now();
        chrono::duration<double> diff = end-start;
        cout << "CP time " << methodDecl->getNameAsString() << " : "
             << diff.count() << endl;
        cout << "---------------------------------------" << endl;
    }
    
    // Remove unused variable definition statements and their declarations
    if (REMOVE_UNUSED_VAR_STMTS()) {
        // Delete unused statements
        travConst.removeUnusedStmt();
        // Delete values from UseDef which related to unused statement removed 
        std::unordered_set<SValue> defVals = travConst.getDefinedVals();
        std::unordered_set<SValue> useVals = travConst.getUsedVals();
        finalState->filterUseDef(defVals, useVals);
    }
    
    // Check for empty process and return empty process code
    if (travConst.getLiveStmts().empty()) {
        return VerilogProcCode(true);
    }
    
    
//    bool first = true;
//    for (auto entry : travConst.getConstEvalFuncs()) {
//        if (!entry.second.isInteger()) continue;
//        
//        if (first) {
//            cout << "METHOD " << methodDecl->getNameAsString() << endl;
//            cout << "   functions evaluated as constant :" << endl;
//        }
//        first = false;
//
//        auto callExpr = dyn_cast<const CallExpr>(entry.first.back());
//        cout << "    " << callExpr->getDirectCallee()->getNameAsString() 
//             << " " << entry.second << endl;
//    }

    // Print state after CPA
    //finalState->print();
//    static int aaa = 1;
//    if (aaa) {
//        globalState->print();
//        aaa = 0;
//    }
    
    // All the variables used in this method process
    unordered_set<SValue> useVals;
    for (const auto& sval : finalState->getReadValues()) {
        useVals.insert(sval);
    }
    
    // Do not generate variable declarations for non-zero elements of MIF array
    bool noneZeroElmntMIF = travConst.isNonZeroElmtMIF();

    // All the variables defined/used in this method process
    defVals.clear();
    InsertionOrderSet<SValue> useDefVals;
    for (const auto& sval : finalState->getDefArrayValues()) {
        defVals.insert(sval);
        useDefVals.insert(sval);
    }
    
//    finalState->print();
//    cout << "------- useVals" << endl;
//    for (const auto& v : useVals) {
//        cout << "   " << v << endl;
//    }
//    cout << "------- defVals" << endl;
//    for (const auto& v : defVals) {
//        cout << "   " << v << endl;
//    }

    // Add read values which are not defined to create local variable 
    // It needs to generate correct code
    for (const auto& sval : finalState->getReadNotDefinedValues()) {
        // Skip constants, channels and pointers to channel
        QualType type = sval.getType();
        if (type.isConstQualified() || isPointerToConst(type) ||
            isScChannel(type) || isScVector(type) || 
            isScChannelArray(type) || isScToolCombSignal(type)) continue;

        // Skip null pointer
        if (isPointer(type)) {
            SValue rval = globalState->getValue(sval);
            if (rval.isInteger() && rval.getInteger().isZero())
                continue;
        }

        if (!useDefVals.count(sval)) {
            //cout << "RND sval " << sval << endl;
            useDefVals.insert(sval);
        }
    }
    
    // Clear initialization values for modified member variables
    clearDefinedVars(procView, dynmodval, defVals);

    // Report read not initialized warning
    for (const auto& sval : finalState->getReadNotInitValues()) {
        // Skip constants, channels and pointers to channel
        QualType type = sval.getType();
        if (type.isConstQualified() || isConstReference(type) || 
            isPointerToConst(type) ||
            isScChannel(type) || isScVector(type) || 
            isScChannelArray(type) || isScToolCombSignal(type)) continue;

        // Skip null pointer
        if (isPointer(type)) {
            SValue rval = globalState->getValue(sval);
            if (rval.isInteger() && rval.getInteger().isZero())
                continue;
        }

        // Do not report for record fields
        if (finalState->isRecField(sval)) continue;
        
        // Do no need to skip members generated as @localparam

        // Do not check duplicates
        ScDiag::reportScDiag(methodDecl->getBeginLoc(), 
                ScDiag::CPP_READ_NOTDEF_VAR, false) << sval.asString(false);
    }

//        cout << "------- useDefVals" << endl;
//        for (const auto& v : useDefVals) {
//            cout << "   " << v << endl;
//        }
//    cout << "------- defVals" << endl;
//    for (const auto& v : defVals) {
//        cout << "   " << v << endl;
//    }
    
    // Register used values
    for (SValue val : useVals) {
        // Replace value to array first element
        SValue zeroVal = finalState->getFirstArrayElementForAny(
                                    val, ScState::MIF_CROSS_NUM);

        // Class field must be in this map, no local variables there
        if (auto elabObj = globalState->getElabObject(zeroVal)) {
            //cout << "ScProcAnalyzer useVal " << val << " elabObj " << elabObj->getDebugString() << endl;
            // Get first array element for non-first element and dereference 
            // pointer to get Verilog variable name
            if (!elabObj) {
                SCT_TOOL_ASSERT (false, "No elaboration object for variable");
                continue;
            }
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }

            if (elabObj->isChannel()) {
                while (!zeroVal.isScChannel() && !zeroVal.isUnknown()) {
                    zeroVal = globalState->getValue(zeroVal);
                }
                SCT_TOOL_ASSERT (zeroVal.isScChannel(), "No channel found");
            }

            bool isChannel = zeroVal.isScChannel();
            bool isConst = !isChannel && (zeroVal.getType().isConstQualified()||
                                          isPointerToConst(zeroVal.getType()));
            bool isNullPtr = false;
            bool isDanglPtr = false;

            if (isPointer(val.getType())) {
                SValue rval = globalState->getValue(val);
                isNullPtr = rval.isInteger() && 
                            rval.getInteger().isZero();
                isDanglPtr = rval.isUnknown();
            }

            auto verVars = verMod->getVerVariables(*elabObj);
            bool isRecord = elabObj->isRecord();
            bool isRecChan = val.isVariable() && 
                             val.getVariable().getParent().isScChannel(); 

            // Constant dangling/null pointer and record/record channel have no variable
            if (verVars.empty() && !isNullPtr && !isDanglPtr && 
                                   !isRecord && !isRecChan) {
                std::string err = "No variable for elaboration object "+
                                  elabObj->getDebugString() + " (1)";
                if (zeroVal.isVariable()) {
                    SCT_INTERNAL_ERROR(zeroVal.getVariable().getDecl()->
                                       getBeginLoc(), err);
                } else {
                    SCT_INTERNAL_ERROR_NOLOC(err);
                }
            }

            // Register used values
            bool first = true;
            for (auto* verVar : verVars) {
                if (first) {
                    globalState->putVerilogTraits(val, VerilogVarTraits(
                                         VerilogVarTraits::COMB, 
                                         VerilogVarTraits::AccessPlace::BOTH,
                                         true, true, verVar->isConstant()));
                    globalState->putVerilogTraits(zeroVal, VerilogVarTraits(
                                         VerilogVarTraits::COMB, 
                                         VerilogVarTraits::AccessPlace::BOTH,
                                         true, true, verVar->isConstant()));
                    first = false;
                }

                verMod->addVarUsedInProc(procView, verVar, isConst, isChannel);
                verMod->putValueForVerVar(verVar, zeroVal);
            }
        }
    }

    // Create process local variables and register defined values
    //cout << "useDefVals " << endl;
    for (SValue val : useDefVals) {
        // Replace value to array first element
        //cout << "  val " << val << endl;
        SValue zeroVal = finalState->getFirstArrayElementForAny(
                                    val, ScState::MIF_CROSS_NUM);
        //cout << "zeroval " << zeroVal << endl;

        // Class field must be in this map, no local variables there
        if (auto elabObj = globalState->getElabObject(zeroVal)) {
            //cout << "ScProcAnalyzer defVal " << val << " elabObj " << elabObj->getDebugString() << endl;
            // Get first array element for non-first element and dereference 
            // pointer to get Verilog variable name
            if (!elabObj) {
                SCT_TOOL_ASSERT (false, "No elaboration object for variable");
                continue;
            }
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }

            if (elabObj->isChannel()) {
                while (!zeroVal.isScChannel() && !zeroVal.isUnknown()) {
                    zeroVal = globalState->getValue(zeroVal);
                }
                SCT_TOOL_ASSERT (zeroVal.isScChannel(), "No channel found");
            }

            bool isChannel = zeroVal.isScChannel();
            bool isConst = !isChannel && (zeroVal.getType().isConstQualified()||
                                          isPointerToConst(zeroVal.getType()));
            bool isPtr = sc::isPointer(val.getType());
            bool isNullPtr = false;
            bool isDanglPtr = false;

            if (isPtr) {
                SValue rval = globalState->getValue(val);
                isNullPtr = rval.isInteger() && rval.getInteger().isZero();
                isDanglPtr = rval.isUnknown();
            }

            auto verVars = verMod->getVerVariables(*elabObj);
            bool isRecord = elabObj->isRecord();
            bool isRecChan = val.isVariable() && 
                             val.getVariable().getParent().isScChannel(); 

            // Constant dangling/null pointer and record have no variable
            if (verVars.empty() && !isNullPtr && !isDanglPtr && 
                                   !isRecord && !isRecChan) {
                std::string err = "No variable for elaboration object "+
                                  elabObj->getDebugString() + " (2)";
                if (zeroVal.isVariable()) {
                    SCT_INTERNAL_ERROR(zeroVal.getVariable().getDecl()->
                                       getBeginLoc(), err);
                } else {
                    SCT_INTERNAL_ERROR_NOLOC(err);
                }
            }

            for (auto* verVar : verVars) {
                isConst = isConst || verVar->isConstant();
                verMod->putValueForVerVar(verVar, zeroVal);
            }

            // Skip constant variable it is declared as @localparam
            if (isConst) continue;

            // Declaration of variable at module level
            // No declaration for non-zero elements of MIF array
            if (!noneZeroElmntMIF) {
                if (!elabObj->isChannel()) {
                    // Store the variable to generate before its process
                    for (auto* verVar : verVars) {
                        //cout << "   convertToProcessLocalVar " << verVar->getName() << endl;
                        verMod->convertToProcessLocalVar(verVar, procView);
                    }
                }
            }

            // Register defined values
            bool first = true;
            for (auto* verVar : verVars) {
                if (first) {
                    globalState->putVerilogTraits(val, VerilogVarTraits(
                                         VerilogVarTraits::COMB, 
                                         VerilogVarTraits::AccessPlace::BOTH,
                                         true, true, false));
                    globalState->putVerilogTraits(zeroVal, VerilogVarTraits(
                                         VerilogVarTraits::COMB, 
                                         VerilogVarTraits::AccessPlace::BOTH,
                                         true, true, false));
                    first = false;
                }
                verMod->addVarDefinedInProc(procView, verVar, isConst, isChannel); 
            }
        }
    }
//        cout << "----- Used variables: " << procView.procName << endl;
//        for (const auto& entry : verMod->procUseVars[procView]) {
//            cout << entry.first->getName() << endl;
//        }
//        cout << "----- Defined variables: " << procView.procName << endl;
//        for (const auto& entry : verMod->procDefVars[procView]) {
//            cout << entry.first->getName() << endl;
//        }
    
    // Checking latches in method
    for (const auto& sval : finalState->getDefSomePathValues()) {
        // Class field must be in this map, no local variables there
        if (auto defObj = globalState->getElabObject(sval)) {
            // Skip non-channel variables
            if (!defObj->isChannel()) continue;

            // There is latch(es) in the process, generate full sensitivity
            // events instead of @*
            procView.setHasLatch();
            
            // Report error if the latch is not registered by @sct_assert_latch()
            if (travConst.getAssertLatches().count(sval) == 0) {
                if (sval.isVariable()) {
                    auto valDecl = sval.getVariable().getDecl();
                    ScDiag::reportScDiag(valDecl->getBeginLoc(),
                                         ScDiag::SYNTH_SOME_PATH_DEFINED) << 
                                         sval.asString(false) <<
                                         methodDecl->getNameAsString();
                } else {
                    // Do not check duplicates
                    ScDiag::reportScDiag(methodDecl->getBeginLoc(), 
                                         ScDiag::SYNTH_SOME_PATH_DEFINED, false) << 
                                         sval.asString(false) <<
                                         methodDecl->getNameAsString();
                }
            }
        }
    }
    
    if (DebugOptions::isEnabled(DebugComponent::doGenStmt) ||
        DebugOptions::isEnabled(DebugComponent::doGenTerm) ||
        DebugOptions::isEnabled(DebugComponent::doGenBlock) ||
        DebugOptions::isEnabled(DebugComponent::doGenRTL) ||
        DebugOptions::isEnabled(DebugComponent::doGenCfg)) {
        std::cout << endl << "--------------- METHOD GEN : " 
                  << methodDecl->getNameAsString() << endl;
        cout << "Run process in module " << modval << endl;
    }

    // Name generator with all member names for current module
    UniqueNamesGenerator& nameGen = verMod->getNameGen();
    
    unique_ptr<ScVerilogWriter> procWriter = std::make_unique<ScVerilogWriter>(
                    sm, true, globalState->getExtrValNames(), nameGen,
                    globalState->getVarTraits(), globalState->getWaitNVarName());
    
    // Clone module state for each process
    auto procState = shared_ptr<ScState>(globalState->clone());
    ScTraverseProc travProc(astCtx, procState, modval, procWriter.get(),
                            nullptr, nullptr, true);
    travProc.setTermConds(travConst.getTermConds());
    travProc.setLiveStmts(travConst.getLiveStmts());
    travProc.setLiveTerms(travConst.getLiveTerms());
    travProc.setRemovedArgExprs(travConst.getRemovedArgExprs());
    travProc.setConstEvalFuncs(travConst.getConstEvalFuncs());

    travProc.run(methodDecl, emptySensitivity);
    
    // Print function RTL
    std::ostringstream os;
    travProc.printLocalDeclarations(os);
    travProc.printFunctionBody(os);
    
    if (DebugOptions::isEnabled(DebugComponent::doGenRTL)) {
        std::cout << "---------------------------------------" << endl;
        std::cout << " Function body RTL: " << endl;
        travProc.printLocalDeclarations(cout);
        travProc.printFunctionBody(cout);
        std::cout << "---------------------------------------" << endl;
    }
    
    VerilogProcCode procCode(os.str());

    // Skip MIF non-zero elements to get number of unique statements
    if (!noneZeroElmntMIF) {
        procCode.statStmtNum = travProc.statStmts.size();
        procCode.statTermNum = travProc.statTerms.size();
        procCode.statAsrtNum = travProc.statAsrts.size();
        procCode.statWaitNum = 0;
    }
    
    // Report error for lack/extra sensitive to SS channels
    // Do not report for non-zero MIF elements
    if (!noneZeroElmntMIF) {
        travProc.reportSctChannel(procView, methodDecl->getAsFunction());
    }
    
    return procCode;
}

// Clear initialization for variables which has been defined  
void ScProcAnalyzer::clearDefinedVars(sc_elab::ProcessView& procView,
                                        const SValue& dynmodval,
                                        const std::unordered_set<SValue>& defVals)
{
    using namespace std;
    using namespace sc_elab;
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

sc_elab::VerilogProcCode ScProcAnalyzer::analyzeCthreadProcess(
                    const SValue &modval, const SValue &dynmodval, 
                    sc_elab::ProcessView procView) 
{
    if (DebugOptions::isEnabled(DebugComponent::doModuleBuilder)) {
        std::cout << "  Analyze thread process: "
        << procView.getLocation().first.getType()->getAsCXXRecordDecl()->getNameAsString()
        << " :: " << procView.getLocation().second->getNameAsString() << std::endl;
    }
    
    sc::ThreadBuilder tb{astCtx, elabDB, procView, globalState, 
                         modval, dynmodval};
    
    return tb.run();
}

// Generated SVA property code from module scope SCT_ASSERT
std::string ScProcAnalyzer::analyzeSvaProperties(
                        sc_elab::VerilogModule& verMod, const SValue& modval,
                        const std::vector<const clang::FieldDecl*>& properties) 
{
    if (properties.empty()) return std::string();
    
    if (DebugOptions::isEnabled(DebugComponent::doGenStmt) ||
        DebugOptions::isEnabled(DebugComponent::doGenTerm) ||
        DebugOptions::isEnabled(DebugComponent::doGenBlock) ||
        DebugOptions::isEnabled(DebugComponent::doGenRTL) ||
        DebugOptions::isEnabled(DebugComponent::doGenCfg)) {
        cout << endl << "--------------- SVA GEN " << endl;
        cout << "Generate SVA for module " << modval << endl;
    }
    
    //std::cout << "verMod " << verMod.getName() << " modval " << modval << "\n";

    // Add CPA to add used variables into Verilog module with @addVarUsedInProc()
    auto constState = shared_ptr<ScState>(globalState->clone());
    ScTraverseConst travConst(astCtx, constState, modval,  
                              globalState, &elabDB, nullptr, nullptr, true, true);
    travConst.setModuleSctAssert();
    
    // Add all used variable to state
    for (const FieldDecl* propDecl : properties) {
        // Get actual parent class, required for assertion in base classes
        QualType partype(propDecl->getParent()->getTypeForDecl(), 0);
        SValue propmodval = getBaseClass(modval, partype);
        travConst.setModval(propmodval ? propmodval : modval);

        travConst.parseSvaDecl(propDecl);
    }

    // Register used variables in Verilog module
    for (const auto& rval : constState->getReadValues()) {
        // Replace value to array first element
        SValue val = constState->getFirstArrayElementForAny(
                                        rval, ScState::MIF_CROSS_NUM);
        
        // Class field must be in this map, no local variables there
        if (auto elabObj = globalState->getElabObject(val)) {
            // Get first array element for non-first element and dereference 
            // pointer to get Verilog variable name
            if (!elabObj) {
                SCT_TOOL_ASSERT (false, "No elaboration object for variable");
                continue;
            }
            if (auto elabOwner = elabObj->getVerilogNameOwner()) {
                elabObj = elabOwner;
            }

            if (elabObj->isChannel()) {
                while (!val.isScChannel() && !val.isUnknown()) {
                    val = globalState->getValue(val);
                }
                SCT_TOOL_ASSERT (val.isScChannel(), "No channel found");
            }

            bool isChannel = val.isScChannel();
            bool isConst = !isChannel && (val.getType().isConstQualified()||
                                          isPointerToConst(val.getType()));
            bool isNullPtr = false;
            bool isDanglPtr = false;

            if (isPointer(val.getType())) {
                SValue rval = globalState->getValue(val);
                isNullPtr = rval.isInteger() && 
                            rval.getInteger().isZero();
                isDanglPtr = rval.isUnknown();
            }

            auto verVars = verMod.getVerVariables(*elabObj);
            bool isRecord = elabObj->isRecord();
            bool isRecChan = val.isVariable() && 
                             val.getVariable().getParent().isScChannel(); 

            // Constant dangling/null pointer and record have no variable
            if (verVars.empty() && !isNullPtr && !isDanglPtr 
                                && !isRecord && !isRecChan) {
                std::string err = "No variable for elaboration object "+
                                  elabObj->getDebugString() + " (3)";
                if (val.isVariable()) {
                    SCT_INTERNAL_ERROR(val.getVariable().getDecl()->
                                       getBeginLoc(), err);
                } else {
                    SCT_INTERNAL_ERROR_NOLOC(err);
                }
            }

            // Register used values
            for (auto* verVar : verVars) {
                verMod.addVarUsedInSva(verVar, isConst, isChannel);
            }
        }
    }
    
    // Name generator with all member names for current module
    UniqueNamesGenerator& nameGen = verMod.getNameGen();
    
    unique_ptr<ScVerilogWriter> procWriter = std::make_unique<ScVerilogWriter>(
                    sm, true, globalState->getExtrValNames(), nameGen,
                    globalState->getVarTraits(), globalState->getWaitNVarName());
    
    // Clone module state for each process
    auto procState = shared_ptr<ScState>(globalState->clone());
    ScTraverseProc travProc(astCtx, procState, modval, procWriter.get(),
                            nullptr, nullptr, true);
    travProc.setTermConds(travConst.getTermConds());
    
    // Generate all properties
    std::string propStr;
    for (const FieldDecl* propDecl : properties) {
        // Get actual parent class, required for assertion in base classes
        QualType partype(propDecl->getParent()->getTypeForDecl(), 0);
        SValue propmodval = getBaseClass(modval, partype);
        propmodval = propmodval ? propmodval : modval;
        travProc.setModval(propmodval);
        // Setup first non-MIF module value
        SValue synmodval = globalState->getSynthModuleValue(
                                        propmodval, ScState::MIF_CROSS_NUM);
        travProc.setSynModval(synmodval);
        
        if (auto code = travProc.runSvaDecl(propDecl)) {
            propStr += *code;
        }
    }

    return propStr;
}

} // namespace sc

