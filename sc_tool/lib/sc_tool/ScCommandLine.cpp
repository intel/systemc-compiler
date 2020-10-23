/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "sc_tool/ScCommandLine.h"

using namespace llvm;

cl::opt<std::string> verilogFileName (
    "sv_out",
    cl::desc("Specify SystemVerilog output filename"),
    cl::value_desc("filename"),
    cl::cat(ScToolCategory)
    );

cl::opt<bool> noSvaGenerate (
    "no_sva_generate",
    cl::desc("No SVA generating for sct_assert, SCT_ASSERT and SCT_ASSERT_LOOP"),
    cl::cat(ScToolCategory)
    );

cl::opt<bool> noRemoveExtraCode(
    "no_remove_extra_code",
    cl::desc("No removing unused variable and extra code"),
    cl::cat(ScToolCategory)
);

cl::opt<bool> initLocalVars(
    "init_local_vars",
    cl::desc("Initialize local variables at declaration with zero"),
    cl::cat(ScToolCategory)
);

cl::opt<bool> keepConstVariables(
    "keep_const_variables",
    cl::desc("No replacing constant variables with values"),
    cl::cat(ScToolCategory)
);


cl::opt<bool> singleBlockCThreads(
    "single_block_cthread",
    cl::desc("Generate single always_ff block for clocked thread"),
    cl::cat(ScToolCategory)
);

cl::opt<bool> noProcessAnalysis (
    "elab_only",
    cl::desc("Elaboration only, disable SystemC process analysis"),
    cl::cat(ScToolCategory)
    );

cl::opt<bool> constPropOnly (
    "const_prop",
    cl::desc("Perform constant propagation test, without generating anything"),
    cl::cat(ScToolCategory)
    );


