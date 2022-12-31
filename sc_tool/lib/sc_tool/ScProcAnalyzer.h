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

#ifndef SCTOOL_SCPROCANALYZER_H
#define SCTOOL_SCPROCANALYZER_H

#include "sc_tool/scope/ScVerilogWriter.h"
#include "sc_tool/cfg/ScState.h"
#include "sc_tool/elab/ScObjectView.h"
#include "sc_tool/elab/ScElabDatabase.h"
#include "sc_tool/elab/ScVerilogModule.h"
#include <fstream>

namespace sc {

/// SystemC process analyzer
class ScProcAnalyzer {

    const clang::ASTContext &astCtx;
    const clang::SourceManager &sm;
    std::shared_ptr<ScState> globalState;
    sc_elab::ElabDatabase &elabDB;

public:
    /// @state is owned outside (by ScElabProcBuilder)
    ScProcAnalyzer(const clang::ASTContext &context,
                   sc_elab::ElabDatabase &elabDB,
                   std::shared_ptr<ScState>  state_) :
        astCtx(context), 
        sm(context.getSourceManager()), 
        globalState(state_),
        elabDB(elabDB) 
    {}
        
    /// Analyze process body and print equivalent Verilog.
    /// \param modval -- current module/class, may be base class value
    /// \param procView -- updated inside, @latchInMethod is set
    /// \return generated Verilog code
    sc_elab::VerilogProcCode analyzeMethodProcess(
                        const SValue& modval,
                        const SValue& dynmodval,
                        sc_elab::ProcessView& procView);

    /// Analyze CTHREAD process body and print equivalent Verilog.
    /// \param modval -- current module/class, may be base class value
    /// \param dynmodval -- dynamic class of the module (most child class type)
    /// \param methodDecl -- method declaration with body
    /// \return generated Verilog code
    sc_elab::VerilogProcCode   analyzeCthreadProcess(
                        const SValue& modval,
                        const SValue& dynmodval,
                        sc_elab::ProcessView procView);
    
    /// Clear initialization for variables which has been defined  
    void clearDefinedVars(sc_elab::ProcessView& procView,
                            const SValue& dynmodval,
                            const std::unordered_set<SValue>& defVals);

    /// Generated SVA property code from module scope SCT_ASSERT
    std::string analyzeSvaProperties(
                        sc_elab::VerilogModule& verMod, 
                        const SValue& modval,
                        const std::vector<const clang::FieldDecl*>& properties);
    
};

}
#endif //SCTOOL_SCPROCANALYZER_H
