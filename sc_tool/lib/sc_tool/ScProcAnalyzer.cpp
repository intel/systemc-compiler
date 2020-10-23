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
#include "sc_tool/cthread/ScCThreadStates.h"
#include "utils/CheckCppInheritance.h"
#include <sc_tool/cthread/ScThreadBuilder.h>
#include <sc_tool/elab/ScObjectView.h>
#include <sc_tool/utils/CfgFabric.h>
#include <sc_tool/utils/DebugOptions.h>
#include <sc_tool/utils/CppTypeTraits.h>
#include <sc_tool/utils/InsertionOrderSet.h>
#include <clang/Analysis/CFG.h>
#include <iostream>
#include <chrono>
#include <sstream>

namespace sc {
    
using namespace clang;
using namespace std;

std::string ScProcAnalyzer::analyzeMethodProcess (
                    const SValue& modval,
                    const SValue& dynmodval, 
                    sc_elab::ProcessView& procView)
{
    using namespace sc_elab;

    if (DebugOptions::isEnabled(DebugComponent::doModuleBuilder)) {
        std::cout << "   Analyze method process: "
        << procView.getLocation().first.getType()->getAsCXXRecordDecl()->getNameAsString()
        << " :: " << procView.getLocation().second->getNameAsString() << std::endl;
    }
    
    clang::CXXMethodDecl* methodDecl = procView.getLocation().second;
    bool emptySensitivity = procView.staticSensitivity().empty();
    bool isCombMethod = procView.isCombinational();
    
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
    
    auto start = chrono::system_clock::now();
    auto constState = shared_ptr<ScState>(globalState->clone());
    ScTraverseConst travConst(astCtx, constState, modval, globalState, 
                              &elabDB, nullptr, true);
    travConst.run(methodDecl);
    const ScState* finalState = travConst.getFinalState();
    
    if (DebugOptions::isEnabled(DebugComponent::doConstProfile)) {
        auto end = chrono::system_clock::now();
        chrono::duration<double> diff = end-start;
        cout << "CP time " << methodDecl->getNameAsString() << " : "
             << diff.count() << endl;
        cout << "---------------------------------------" << endl;
    }
    
    // Print state after CPA
    //finalState->print();

    // All the variables used in this method process
    unordered_set<SValue> useVals;
    for (const auto& sval : finalState->getReadValues()) {
        useVals.insert(sval);
    }
    
//    cout << "------- useVals" << endl;
//    for (const auto& v : useVals) {
//        cout << "   " << v << endl;
//    }
    
    // Do not generate variable declarations for non-zero elements of MIF array
    if (!singleBlockCThreads && !travConst.isNonZeroElmtMIF()) {
        //cout << "----- finalState->getReadValues() size " 
        //     << finalState->getReadValues().size() << endl;
        
        // All the variables defined/used in this method process
        InsertionOrderSet<SValue> useDefVals;
        for (const auto& sval : finalState->getDefArrayValues()) {
            useDefVals.insert(sval);
            
            // Check member constant variables not defined
            if (sval.isVariable() && sval.getType().isConstQualified()) {
                const ValueDecl* valDecl = sval.getVariable().getDecl();
                const DeclContext* declContext = valDecl->getDeclContext();
                bool isLocalVar = isa<FunctionDecl>(declContext);
                
                if (!isLocalVar) {
                    ScDiag::reportScDiag(methodDecl->getBeginLoc(), 
                        ScDiag::SYNTH_CONST_VAR_MODIFIED) << sval.asString(false);
                }
            }
        }
        
        // Add read values which are not defined to create local variable 
        // It needs to generated correct code
        for (const auto& sval : finalState->getReadNotDefinedValues()) {
            // Skip constants, channels and pointers to channel
            QualType type = sval.getType();
            if (type.isConstQualified() || isPointerToConst(type) ||
                isScChannel(type) || isScVector(type) || 
                isScChannelArray(type) || isScToolCombSignal(type)) continue;
            
            // Skip null pointer
            if (isPointer(type)) {
                SValue rval = globalState->getValue(sval);
                if (rval.isInteger() && rval.getInteger().isNullValue())
                    continue;
            }
            
            if (!useDefVals.count(sval)) {
                //cout << "RND sval " << sval << endl;
                useDefVals.insert(sval);
            }
        }
        
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
                if (rval.isInteger() && rval.getInteger().isNullValue())
                    continue;
            }
            
            //cout << "RND sval " << sval << endl;
            std::string varName = sval.isVariable() ? 
                                  sval.getVariable().asString(false) : "---";
            // Do not check duplicates
            ScDiag::reportScDiag(methodDecl->getBeginLoc(), 
                                 ScDiag::CPP_READ_NOTDEF_VAR, false) << varName;
        }
        
//        cout << "------- useDefVals" << endl;
//        for (const auto& v : useDefVals) {
//            cout << "   " << v << endl;
//        }
        
        // Register used values
        for (SValue val : useVals) {
            // Replace value to array first element
            val = finalState->getFirstArrayElementForAny(
                                        val, ScState::MIF_CROSS_NUM);
            
            // Class field must be in this map, no local variables there
            if (auto elabObj = globalState->getElabObject(val)) {
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
                                rval.getInteger().isNullValue();
                    isDanglPtr = rval.isUnknown();
                }

                auto verVars = verMod->getVerVariables(*elabObj);
                bool isRecord = elabObj->isRecord();

                // Constant dangling/null pointer and record have no variable
                if (verVars.empty() && !isNullPtr && !isDanglPtr && !isRecord) {
                    std::string err = "No variable for elaboration object "+
                                      elabObj->getDebugString() + " (1)";
                    if (val.isVariable()) {
                        SCT_INTERNAL_ERROR(val.getVariable().getDecl()->
                                           getBeginLoc(), err);
                    } else {
                        SCT_INTERNAL_ERROR_NOLOC(err);
                    }
                }
                
                // Register used values
                for (auto* verVar : verVars) {
                    verMod->addVarUsedInProc(procView, verVar, isConst, isChannel);
                    verMod->putValueForVerVar(verVar, val);
                }
            }            
        }
        
        // Create process local variables and register defined values
        for (SValue val : useDefVals) {
            // Replace value to array first element
            val = finalState->getFirstArrayElementForAny(
                                        val, ScState::MIF_CROSS_NUM);
            
            // Class field must be in this map, no local variables there
            if (auto elabObj = globalState->getElabObject(val)) {
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
                    isNullPtr = rval.isInteger() && rval.getInteger().isNullValue();
                    isDanglPtr = rval.isUnknown();
                }
                
                auto verVars = verMod->getVerVariables(*elabObj);
                bool isRecord = elabObj->isRecord();

                // Constant dangling/null pointer and record have no variable
                if (verVars.empty() && !isNullPtr && !isDanglPtr && !isRecord) {
                    std::string err = "No variable for elaboration object "+
                                      elabObj->getDebugString() + " (2)";
                    if (val.isVariable()) {
                        SCT_INTERNAL_ERROR(val.getVariable().getDecl()->
                                           getBeginLoc(), err);
                    } else {
                        SCT_INTERNAL_ERROR_NOLOC(err);
                    }
                }
                
                for (auto* verVar : verVars) {
                    isConst = isConst || verVar->isConstant();
                    verMod->putValueForVerVar(verVar, val);
                }

                // Skip constant variable it is declared as @localparam
                if (isConst) continue;
                
                if (!elabObj->isChannel()) {
                    // Store the variable to generate before its process
                    for (auto* verVar : verVars) {
                        //cout << "   convertToProcessLocalVar " << verVar->getName() << endl;
                        verMod->convertToProcessLocalVar(verVar, procView);
                    }
                }
                
                // Register defined values
                for (auto* verVar : verVars) {
                    verMod->addVarDefinedInProc(procView, verVar, isConst, 
                                                isChannel); 
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
    }
    
    // Checking method sensitivity list is complete
    for (const auto& readVal : useVals) {
        // Class field must be in this map, no local variables there
        if (auto readObj = globalState->getElabObject(readVal)) {
            // Skip non-channel variables
            if (!readObj->isChannel()) continue;
                
            bool found = false;
            for (const auto& event : procView.staticSensitivity()) {
                if (readObj.getValue() == event.sourceObj && event.isDefault()) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                ScDiag::reportScDiag(methodDecl->getBeginLoc(), 
                                     ScDiag::SYNTH_NON_SENSTIV_2USED) << 
                                     methodDecl->getNameAsString() <<
                                     readVal.asString(false);
            }
        }
    }
    
    // Checking latches in method
    for (const auto &sval : finalState->getDefSomePathValues()) {
        // Class field must be in this map, no local variables there
        if (auto defObj = globalState->getElabObject(sval)) {
            // Skip non-channel variables
            if (!defObj->isChannel()) continue;

            // There is latch(es) in the process, generate full sensitivity
            // events instead of @*
            procView.setHasLatch();
            
            // Report error if the latch is not registered by @sct_assert_latch()
            if (travConst.getAssertLatches().count(sval) == 0) {
                ScDiag::reportScDiag(methodDecl->getBeginLoc(),
                                     ScDiag::SYNTH_SOME_PATH_DEFINED) << 
                                     sval.asString(false) <<
                                     methodDecl->getNameAsString();
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

    // Create writer from module writer to keep variable indices 
    // Update external names from constant propagator, required for static constants
    modWriter.updateExtrNames(globalState->getExtrValNames());
    shared_ptr<ScVerilogWriter> writer(new ScVerilogWriter(modWriter));
    
    // Clone module state for each process
    auto procState = shared_ptr<ScState>(globalState->clone());
    ScTraverseProc travProc(astCtx, procState, modval, writer.get(), true,
                            nullptr, nullptr, isCombMethod);
    travProc.setTermConds(travConst.getTermConds());
    travProc.run(methodDecl, emptySensitivity);
    
    // Add constants not replaced with integer value, 
    // used to determine required variables
    for (const SValue& val : writer->getNotReplacedVars()) {
        verMod->addNotReplacedVar(val);
    }
    
    // Add empty sensitive METHOD local variable names to module writer
    modWriter.updateExtrNames(writer->getExtrNames());
    
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
    return os.str();
}

sc_elab::VerilogProcCode ScProcAnalyzer::analyzeCthreadProcess(
                    const SValue &modval, const SValue &dynmodval, 
                    sc_elab::ProcessView procView) 
{
    if (DebugOptions::isEnabled(DebugComponent::doModuleBuilder)) {
        std::cout << "   Analyze thread process: "
        << procView.getLocation().first.getType()->getAsCXXRecordDecl()->getNameAsString()
        << " :: " << procView.getLocation().second->getNameAsString() << std::endl;
    }
    
    sc::ThreadBuilder tb{astCtx, elabDB, procView, globalState, 
                         modval, dynmodval};
    
    return tb.run(false);
}

void ScProcAnalyzer::analyzeConstProp(const SValue &modval,
                                      const SValue &dynmodval,
                                      sc_elab::ProcessView procView) {
    sc::ThreadBuilder tb{astCtx, elabDB, procView, globalState, modval, dynmodval};
    tb.run(true);
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
    ScTraverseConst travConst(astCtx, constState, modval, globalState, 
                              &elabDB, nullptr, true);
    
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
                            rval.getInteger().isNullValue();
                isDanglPtr = rval.isUnknown();
            }

            auto verVars = verMod.getVerVariables(*elabObj);
            bool isRecord = elabObj->isRecord();

            // Constant dangling/null pointer and record have no variable
            if (verVars.empty() && !isNullPtr && !isDanglPtr && !isRecord) {
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
    
    // Create writer from module writer to keep variable indices 
    // Update external names from CPA, required for static constants
    modWriter.updateExtrNames(globalState->getExtrValNames());
    shared_ptr<ScVerilogWriter> writer(new ScVerilogWriter(modWriter));
    
    // Clone module state for each process
    auto procState = shared_ptr<ScState>(globalState->clone());
    ScTraverseProc travProc(astCtx, procState, modval, writer.get(), true,
                            nullptr, nullptr, true);
    
    // Generate all properties
    std::string propStr;
    for (const FieldDecl* propDecl : properties) {
        // Get actual parent class, required for assertion in base classes
        QualType partype(propDecl->getParent()->getTypeForDecl(), 0);
        SValue propmodval = getBaseClass(modval, partype);
        travProc.setModval(propmodval ? propmodval : modval);
        
        if (auto code = travProc.runSvaDecl(propDecl)) {
            propStr += *code;
        }
    }

    return propStr;
}

} // namespace sc

