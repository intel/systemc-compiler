/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCTHREADVERILOGGEN_H
#define SCTOOL_SCTHREADVERILOGGEN_H

#include "sc_tool/elab/ScVerilogModule.h"
#include <sc_tool/cfg/ScTraverseProc.h>
#include <sc_tool/cfg/ScTraverseConst.h>
#include <sc_tool/scope/ScVerilogWriter.h>
#include <sc_tool/elab/ScObjectView.h>
#include <sc_tool/cthread/ScCThreadStates.h>
#include <sc_tool/cfg/ScState.h>
#include <sc_tool/ScCommandLine.h>
#include <sc_tool/utils/RawIndentOstream.h>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/Analysis/CFG.h>

#include <unordered_map>

namespace sc {

/// Verilog generator for CTHREAD FSM top-level case
class ScThreadVerilogGen {

    const sc_elab::ProcessView procView;
    clang::PrintingPolicy prntPol;

    const ScCThreadStates &cthreadStates;

    std::unordered_map<WaitID, std::string> stateCodeMap;
    
    const clang::ASTContext &astCtx;
    const FindWaitCallVisitor &findWait;
    const SValue& modval;

    /// Process state  
    std::shared_ptr<ScState> state;
    
    /// Code writer for module non-local variables, contains variable name
    /// indices and used as initial copy for process writer class  
    ScVerilogWriter modWriter;
    /// Traverse process
    ScTraverseProc travProc;
    /// Current thread has reset signal
    bool hasReset;
    /// Thread without a PROC_STATE variable
    bool isSingleStateThread = false;
    
public:
    ScThreadVerilogGen(sc_elab::ProcessView procView,
                       const clang::ASTContext &astCtx_,
                       const ScCThreadStates &cthreadStates_,
                       ScState &state_,
                       const FindWaitCallVisitor &findWait_,
                       const SValue &modval_,
                       const SValue &dynmodval,
                       bool isSingleStateThread_,
                       const std::pair<std::string, std::string>& stateRegNames,
                       const std::pair<std::string, std::string>& waitNRegNames
                       ) :
        procView(procView),
        prntPol(astCtx_.getLangOpts()),
        cthreadStates(cthreadStates_),
        astCtx(astCtx_),
        findWait(findWait_),
        modval(modval_),
        state(new ScState(state_)),
        modWriter(astCtx_.getSourceManager(), false, state->getExtrValNames(),
                  state->getVarTraits(), state->getWaitNVarName()),
        travProc(astCtx_, state, modval_, &modWriter,
                 &cthreadStates_, &findWait_, false, isSingleStateThread_),
        hasReset(!procView.resets().empty()),
        isSingleStateThread(isSingleStateThread_)
    {
        if (!singleBlockCThreads) {
            STATE_REG_NAMES  = stateRegNames;
            WAIT_N_REG_NAMES = waitNRegNames;
        }
    }

    std::pair<std::string, std::string> STATE_REG_NAMES;
    std::pair<std::string, std::string> WAIT_N_REG_NAMES;

    // Get process Verilog code
    sc_elab::VerilogProcCode getVerilogCode();

    ScProcContext getInitialTraverseProcState(){
        return getInitialTraverseContext();
    }

    std::vector<std::pair<WaitID, ScProcContext>> generateCodePath(
            ScProcContext traverseContext, WaitID startStateID,
            const ScTraverseConst& constProp);
    
    /// Get context for function entry
    ScProcContext getInitialTraverseContext();
    
    /// @return traverse context for each reachable state
    /// \param startStateID -- start state
    /// \param constProp    -- global constant propagation
    /// \param os           -- stream for generated code
    std::vector<std::pair<WaitID, ScProcContext> > generateCodeForPath (
        const ScProcContext & traverseContext,
        WaitID startStateID,
        const ScTraverseConst& constProp,
        std::ostream &os);


    void generateLocalVariables (std::ostream &os);
    void generateResetCombVariables (std::ostream &os);
    
    /// Get global constant not replaced by integer values
    inline std::unordered_set<SValue> getNotReplacedVars() {
        return modWriter.getNotReplacedVars();
    }
};

}

#endif //SCTOOL_SCTHREADVERILOGGEN_H
