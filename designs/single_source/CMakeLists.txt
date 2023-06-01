#******************************************************************************
# Copyright (c) 2023, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

## Simple tests with Target and Initiator
add_executable(ss_simple_method sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_method PUBLIC -DMETHOD)
svc_target(ss_simple_method ELAB_TOP test_top.dut GOLDEN ss_simple_method.sv)

add_executable(ss_simple_thread sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_thread PUBLIC -DTHREAD)
svc_target(ss_simple_thread ELAB_TOP test_top.dut GOLDEN ss_simple_thread.sv)

add_executable(ss_simple_bput sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_bput PUBLIC -DBPUT)
svc_target(ss_simple_bput ELAB_TOP test_top.dut GOLDEN ss_simple_bput.sv)

add_executable(ss_simple_targ_vect sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_targ_vect PUBLIC -DTARG_VECT)
svc_target(ss_simple_targ_vect ELAB_TOP test_top.dut.a GOLDEN ss_simple_targ_vect.sv)


## FIFO used for interprocess communications
add_executable(ss_fifo_shared_meth sct_fifo_shared/sc_main.cpp)
target_compile_definitions(ss_fifo_shared_meth PUBLIC -DMETHOD)
svc_target(ss_fifo_shared_meth ELAB_TOP test_top.dut GOLDEN ss_fifo_shared_meth.sv)

add_executable(ss_fifo_shared_thread sct_fifo_shared/sc_main.cpp)
target_compile_definitions(ss_fifo_shared_thread PUBLIC -DTHREAD)
svc_target(ss_fifo_shared_thread ELAB_TOP test_top.dut GOLDEN ss_fifo_shared_thread.sv)
