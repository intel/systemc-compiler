#******************************************************************************
# Copyright (c) 2023, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

set(COMMON_TESTS $ENV{ICSC_HOME}/icsc/components/common/test)

## Simple tests with Target and Initiator
add_executable(ss_simple_method ${COMMON_TESTS}/sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_method PUBLIC -DMETHOD)
svc_target(ss_simple_method ELAB_TOP test_top.dut GOLDEN ss_simple_method.sv)

add_executable(ss_simple_thread ${COMMON_TESTS}/sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_thread PUBLIC -DTHREAD)
svc_target(ss_simple_thread ELAB_TOP test_top.dut GOLDEN ss_simple_thread.sv)

add_executable(ss_simple_bput ${COMMON_TESTS}/sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_bput PUBLIC -DBPUT)
svc_target(ss_simple_bput ELAB_TOP test_top.dut GOLDEN ss_simple_bput.sv)

add_executable(ss_simple_targ_vect ${COMMON_TESTS}/sct_simple/sc_main.cpp)
target_compile_definitions(ss_simple_targ_vect PUBLIC -DTARG_VECT)
svc_target(ss_simple_targ_vect ELAB_TOP test_top.dut.a GOLDEN ss_simple_targ_vect.sv)


## FIFO used for interprocess communications
add_executable(ss_fifo_shared_meth ${COMMON_TESTS}/sct_fifo_shared/sc_main.cpp)
target_compile_definitions(ss_fifo_shared_meth PUBLIC -DMETHOD)
svc_target(ss_fifo_shared_meth ELAB_TOP test_top.dut GOLDEN ss_fifo_shared_meth.sv)

add_executable(ss_fifo_shared_thread ${COMMON_TESTS}/sct_fifo_shared/sc_main.cpp)
target_compile_definitions(ss_fifo_shared_thread PUBLIC -DTHREAD)
svc_target(ss_fifo_shared_thread ELAB_TOP test_top.dut GOLDEN ss_fifo_shared_thread.sv)
