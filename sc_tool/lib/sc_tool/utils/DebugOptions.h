/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Options to adjust debug information printed
 * 
 * Author: Roman Popov
 */

#ifndef DEBUGOPTIONS_H
#define DEBUGOPTIONS_H

namespace sc {

/*
 * Command Line:
 * Enable all debug output:
 *    sim_exe -sctool -debug
 *
 * Enable only specified components
 *    sim_exe -sctool -debug-only=doConstCfg,doGenLoop
 *
 *
 * Code usage Example:
 *
 * if (DebugOptions::isEnabled(DebugOptions::doElabState)) {
 *    // some debug output
 * }
 *
 *
 * DEBUG_WITH_TYPE(DebugOptions::doGenLoop, outs() << "My stderr output");
 *
 * DEBUG_WITH_TYPE(DebugOptions::doGenLoop, llvm::outs() << "My stdout output");
 *
 */

namespace DebugOptions {

#define DEBUG_OPT(optName) \
    inline static const char * const optName = #optName

    DEBUG_OPT(doElab);
    DEBUG_OPT(doElabState);
    DEBUG_OPT(doPortBind);
    DEBUG_OPT(doModuleBuilder);
    DEBUG_OPT(doProcBuilder);
    DEBUG_OPT(doThreadBuild);
    DEBUG_OPT(doState);
    DEBUG_OPT(doUseDef);
    DEBUG_OPT(doConstCfg);
    DEBUG_OPT(doConstStmt);
    DEBUG_OPT(doConstTerm);
    DEBUG_OPT(doConstLoop);
    DEBUG_OPT(doConstFuncCall);
    DEBUG_OPT(doGenFuncCall);
    DEBUG_OPT(doConstBlock);
    DEBUG_OPT(doConstResult);
    DEBUG_OPT(doConstProfile);
    DEBUG_OPT(doGenState);
    DEBUG_OPT(doGenStmt);
    DEBUG_OPT(doGenTerm);
    DEBUG_OPT(doGenLoop);
    DEBUG_OPT(doGenBlock);
    DEBUG_OPT(doGenCfg);
    DEBUG_OPT(doGenRTL);
    DEBUG_OPT(doGenName);
    DEBUG_OPT(doVerWriter);
    DEBUG_OPT(doScopeGraph);

#undef DEBUG_OPT

    bool isEnabled(const char* optName);
    
    void enable(const char **optNames, unsigned count);

    bool isDebug();
    
    void suspend();

    void resume();
}

namespace DebugComponent = DebugOptions;

}

#endif /* DEBUGOPTIONS_H */

