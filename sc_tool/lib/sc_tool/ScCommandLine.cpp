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
    cl::desc("Generated SystemVerilog file name"),
    cl::value_desc("filename"),
    cl::cat(ScToolCategory)
    );

cl::opt<bool> noSvaGenerate (
    "no_sva_generate",
    cl::desc("No SVA generating for sct_assert, SCT_ASSERT and SCT_ASSERT_LOOP"),
    cl::cat(ScToolCategory)
    );

cl::opt<bool> portMapGenerate (
    "portmap_generate",
    cl::desc("Generate port map file and top module wrapper"),
    cl::cat(ScToolCategory)
    );

cl::opt<bool> noRemoveExtraCode(
    "no_remove_extra_code",
    cl::desc("No removing unused variable and extra code"),
    cl::cat(ScToolCategory)
);

cl::opt<bool> checkUnsigned(
    "check_unsigned",
    cl::desc("Design uses unsigned arithmetic only"),
    cl::cat(ScToolCategory)
);

cl::opt<bool> initLocalVars(
    "init_local_vars",
    cl::desc("Initialize local variables at declaration with zero"),
    cl::cat(ScToolCategory)
);

cl::opt<bool> initResetLocalVars(
    "init_reset_local_vars",
    cl::desc("Initialize CTHREAD reset local variables at declaration with zero"),
    cl::cat(ScToolCategory)
);

cl::opt<std::string> modulePrefix (
    "module_prefix",
    cl::desc("Module prefix string"),
    cl::cat(ScToolCategory)
    );


