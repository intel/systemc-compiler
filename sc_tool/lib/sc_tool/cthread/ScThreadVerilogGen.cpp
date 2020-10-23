/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "ScThreadVerilogGen.h"


#include <sc_tool/elab/ScVerilogModule.h>
#include <sc_tool/utils/BitUtils.h>
#include "sc_tool/utils/DebugOptions.h"
#include <sc_tool/ScCommandLine.h>
#include <clang/AST/Expr.h>
#include <sstream>

using namespace llvm;
using namespace clang;

namespace sc {

std::vector<std::pair<WaitID, ScProcContext>>
ScThreadVerilogGen::generateCodePath(ScProcContext traverseContext,
                                     WaitID startStateID, 
                                     const ScTraverseConst& constProp)
{
    std::stringstream ss;
    auto res = generateCodeForPath(traverseContext, startStateID, constProp, ss);
    std::string generatedCode = ss.str();

    if (!singleBlockCThreads && startStateID == RESET_ID && 
        !procView.resets().empty()) 
    {
        // Generate local variables of reset section
        std::stringstream ls;
        generateResetCombVariables(ls);
        generatedCode = ls.str() + generatedCode;
    }
    
    SCT_TOOL_ASSERT(stateCodeMap.count(startStateID) == 0, "No state found");
    stateCodeMap[startStateID] = generatedCode;

    return res;
}

std::vector<std::pair<WaitID, ScProcContext> > 
ScThreadVerilogGen::generateCodeForPath(const ScProcContext& traverseContext,
                                      WaitID startStateID, 
                                      const ScTraverseConst& constProp,
                                      std::ostream &os)
{
    using namespace std;
    auto threadDecl = cthreadStates.getEntryFunction();
    if (traverseContext.empty()) {
        
        travProc.setFunction(threadDecl);
        travProc.setTermConds(constProp.getTermConds());
        
    } else {
        travProc.setContextStack(traverseContext);
    }
    travProc.setWaitFuncs(constProp.getWaitFuncs());
    
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
        cout << "Run process in module " << modval << endl;
    }
    // Use @ScTraverseConst results if this line commented
    travProc.setHasReset(hasReset);
    travProc.run();
    
    // Print function RTL
    travProc.printFunctionBody(os);
    
    if (DebugOptions::isEnabled(DebugComponent::doGenRTL)) {
        cout << "---------------------------------------" << endl;
        cout << " Function body RTL: " << endl;
        cout << "---------------------------------------" << endl;
        travProc.printFunctionBody(cout);
        cout << "---------------------------------------" << endl;
    }
    
    // Fill wait contexts
    vector<pair<WaitID, ScProcContext>> waitContexts;
    
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << "Reached wait`s :" << endl;
    }
    for (auto& ctxStack : travProc.getWaitContexts()) {
        WaitID waitId = cthreadStates.getStateID(ctxStack.getCursorStack());
        waitContexts.push_back( {waitId, ctxStack} );
        
        if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
            cout << "   wait #" << waitId << endl;
        }
    }
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << endl;
    }

    return waitContexts;
}

sc_elab::VerilogProcCode ScThreadVerilogGen::getVerilogCode()
{
    std::string localVars;
    std::string caseBody;
    std::string resetSection;

    {
        // Generate Local Variables
        std::stringstream ls;
        generateLocalVariables(ls);
        
        if (singleBlockCThreads) {
            if (cthreadStates.hasWaitNState()) {
                size_t sizeBits = bitsInNumber(cthreadStates.getMaxWaitN());
                ls << "    logic ";
                if (sizeBits > 1)
                    ls << "[" << sizeBits - 1 << ":0] ";
                ls << WAIT_N_REG_NAMES.first << ";\n";
            }

            if (!isSingleStateThread) {
                size_t sizeBits = bitsForIndex(cthreadStates.getNumFSMStates());
                ls << "    logic ";
                if (sizeBits > 1)
                    ls << "[" << sizeBits - 1 << ":0] ";
                ls << STATE_REG_NAMES.first << ";\n";
            } else {
                ls << "\n";
            }
        } else {
            // Add wait(n) counter current to next assignment
            if (cthreadStates.hasWaitNState()) {
                ls << "    " << WAIT_N_REG_NAMES.second << " = " 
                   << WAIT_N_REG_NAMES.first << ";\n";
            }
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
            if (!isSingleStateThread && singleBlockCThreads) {
                vout << "repeat(1) ";
            }
            vout << "begin\n";
            
            vout << stateCodeMap[RESET_ID];

            // Add wait(n) counter initialization
            if (cthreadStates.hasWaitNState()) {
                vout << "    " << WAIT_N_REG_NAMES.first << " <= 0;\n";
            }

            vout << "end\n";
        }

        resetSection = verRawOS.str();
    }

    {
        // Generate @case(PROC_STATE) ...
        std::string verOutStr;
        llvm::raw_string_ostream verRawOS{verOutStr};
        RawIndentOstream vout{verRawOS};

        if (!(isSingleStateThread && !singleBlockCThreads))
            vout.pushF("    ");

        if (!isSingleStateThread) {
            vout << "    " << STATE_REG_NAMES.second << " = " 
                 << STATE_REG_NAMES.first << ";\n";
            
            if (singleBlockCThreads)
                vout << "repeat(1) begin\n";
            else
                vout << "\n";

            vout << "case (" << STATE_REG_NAMES.first << ")\n";
            vout.pushF("    ");
        } else  {
            if (singleBlockCThreads)
                vout << "begin\n";
        }

        if (procView.resets().empty()) {
            vout << "default : begin\n";
            vout << stateCodeMap[RESET_ID];
            vout << "end\n";
        }

        for (size_t waitID = 0; waitID < cthreadStates.getNumFSMStates(); ++waitID) 
        {
            if (!isSingleStateThread) {
                vout << waitID << ": begin\n";
            }

            vout << stateCodeMap[waitID];

            if (!isSingleStateThread) {
                vout << "end\n";
            }
        }

        if (!isSingleStateThread) {
            vout.popF();
            vout << "endcase\n";
        }

        if (singleBlockCThreads)
            vout << "end\n";

        if (!(isSingleStateThread && !singleBlockCThreads))
            vout.popF();

        caseBody = verRawOS.str();
    }
    
    // Get temporal assertions
    std::stringstream ss;
    travProc.printTemporalAsserts(ss, false);
    std::string tempAsserts = ss.str();
    std::stringstream ssr;
    travProc.printTemporalAsserts(ssr, true);
    std::string tempRstAsserts = ssr.str();
    
    //std::cout << "Temporal asserts in proc:\n" << tempAsserts << std::endl;

    return sc_elab::VerilogProcCode(caseBody, localVars, resetSection, 
                                    tempAsserts, tempRstAsserts);

}

ScProcContext ScThreadVerilogGen::getInitialTraverseContext()
{
    return ScProcContext();
}

void ScThreadVerilogGen::generateLocalVariables(std::ostream &os)
{
    travProc.printLocalDeclarations(os);
}

void ScThreadVerilogGen::generateResetCombVariables (std::ostream &os)
{
    travProc.printResetDeclarations(os);
}

} // end namespace sc

