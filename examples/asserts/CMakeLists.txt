#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Example immediate assertions (sct_assert)

# All synthesizable source files must be listed here (not in libraries)
add_executable(imm_assert imm_assert.cpp)
add_executable(temp_assert temp_assert.cpp)

# Test source directory
target_include_directories(imm_assert PUBLIC 
    $ENV{ICSC_HOME}/examples/asserts
)
target_include_directories(temp_assert PUBLIC 
    $ENV{ICSC_HOME}/examples/asserts
)

svc_target(imm_assert ELAB_TOP tb_mod.assert_mod)
svc_target(temp_assert ELAB_TOP tb_mod.assert_mod)
