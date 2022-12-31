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

#ifndef SCTOOL_SCTHREADBUILDER_H
#define SCTOOL_SCTHREADBUILDER_H

#include <sc_tool/utils/ScTypeTraits.h>
#include <sc_tool/cthread/ScFindWaitCallVisitor.h>
#include <sc_tool/elab/ScObjectView.h>
#include <sc_tool/utils/RawIndentOstream.h>
#include <sc_tool/utils/InsertionOrderSet.h>
#include <sc_tool/cthread/ScCThreadStates.h>
#include <sc_tool/elab/ScElabDatabase.h>
#include <sc_tool/elab/ScVerilogModule.h>
#include "sc_tool/cfg/ScTraverseProc.h"
#include <sc_tool/cfg/ScTraverseConst.h>
#include <sc_tool/cfg/ScState.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/Analysis/CFG.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <unordered_map>
#include <queue>
#include <unordered_set>

namespace sc {

/**
 *  Thread Builder coordinates SC_THREAD/SC_CTHREAD analysis and code generation
 *
 *  Currently it does following things:
 *  - Runs const propagation for all wait states
 *  - Creates extra variables to model register by pair of X and X_next variables
 *  - Runs Verilog code generation for all wait states
 */
class ThreadBuilder {

public:
    ThreadBuilder(const clang::ASTContext &astCtx,
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

    /**
     * @return generated code
     */
    sc_elab::VerilogProcCode run();

    const clang::CFG * getCurrentCFG(const CfgCursorStack &callStack,
                                     const SValue & dynModVal);

    const clang::FunctionDecl* getCurrentFuncDecl(
                                const CfgCursorStack &callStack, 
                                const SValue & dynModVal);

    /// If callCursor is function call, returns CallExpr, otherwise nullptr
    const clang::CallExpr * getCursorCallExpr(const CfgCursor &callCursor);
    
private:
    
    /// Run constant propagation for a single state (starting from wait() call)
    void runConstPropForState(WaitID stateID);

    /// Analyze UseDef analysis results to create registers
    void analyzeUseDefResults(const ScState* finalState,
                              std::unordered_set<SValue>& useVals, 
                              std::unordered_set<SValue>& defVals);
    
    /// Get initialized variables to skip report read-no-initialized error
    std::unordered_set<SValue> getInitNonDefinedVars(
                            const std::unordered_set<SValue>& useVals,
                            const std::unordered_set<SValue>& defVals);
    /// Clear initialization for variables which has been defined  
    void clearDefinedVars(const std::unordered_set<SValue>& defVals);

    /// Generate Verilog for a single state
    /// Fill @traverseContextMap and return reachable wait IDs
    void generateVerilogForState(WaitID stateID);
    
    /// Generate code for one state
    std::vector<std::pair<WaitID, ScProcContext>>
    generateCodePath(ScProcContext traverseContext, WaitID startStateID);

    /// Get process code including declarations and SVA code
    sc_elab::VerilogProcCode getVerilogCode(bool isSingleState);

    /// Generate Verilog module variables for registers discovered in thread
    void generateThreadLocalVariables();

    const clang::SourceManager &sm;
    const clang::ASTContext &astCtx;
    
    CfgFabric &cfgFab;
    
    const clang::FunctionDecl * entryFuncDecl;
    FindWaitCallVisitor findWaitVisitor;

    sc_elab::ElabDatabase &elabDB;

    std::shared_ptr<sc::ScState> globalState;
    const SValue &modSval;
    const SValue &dynModSval;

    /// Filled by local CPA, contains call stack for wait`s
    ScCThreadStates threadStates;

    /// Registers assigned by thread 
    InsertionOrderSet<SValue> threadRegVars;
    /// Thread-local combinational variables 
    InsertionOrderSet<SValue> threadCombVars;
    /// Read-only constant or channel variables 
    InsertionOrderSet<SValue> threadReadOnlyVars;
    /// Read but not defined non-constant non-channel variable, 
    /// used to report error
    InsertionOrderSet<SValue> threadReadNotDefVars;
    /// Read non-channel variable, used to register the variable as used
    std::unordered_set<SValue> threadReadVars;

    bool isSingleState;

    const sc_elab::ProcessView procView;

    std::unique_ptr<ScTraverseConst> travConst;
    std::unique_ptr<ScTraverseProc>  travProc;

    /// State contexts for @travProc
    std::unordered_map<WaitID, ScProcContext> traverseContextMap;

    /// Code generated for states
    std::unordered_map<WaitID, std::string> stateCodeMap;

    // Name and next name 
    std::pair<std::string, std::string> waitNRegNames;
    std::pair<std::string, std::string> stateRegNames;
};


} // namespace sc

#endif //SCTOOL_SCTHREADBUILDER_H
