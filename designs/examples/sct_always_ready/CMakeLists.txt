#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(sct_always_ready_meth sc_main.cpp)
svc_target(sct_always_ready_meth ELAB_TOP test_top.dut GOLDEN sct_always_ready_meth.sv)

add_executable(sct_always_ready_thread sc_main.cpp)
target_compile_definitions(sct_always_ready_thread PUBLIC -DTHREAD)
svc_target(sct_always_ready_thread ELAB_TOP test_top.dut GOLDEN sct_always_ready_thread.sv)

