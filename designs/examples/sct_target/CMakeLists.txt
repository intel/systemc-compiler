#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

## Target used in METHOD
add_executable(sct_target_method sc_main.cpp)
svc_target(sct_target_method ELAB_TOP test_top.dut GOLDEN sct_target_method.sv)
