#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(sct_fifo_shared sc_main.cpp)
target_compile_definitions(sct_fifo_shared PUBLIC -DSHARED)
svc_target(sct_fifo_shared ELAB_TOP test_top.dut GOLDEN sct_fifo_shared.sv)

add_executable(sct_fifo_single sc_main.cpp)
target_compile_definitions(sct_fifo_single PUBLIC -DSINGLE)
svc_target(sct_fifo_single ELAB_TOP test_top.dut GOLDEN sct_fifo_single.sv)

add_executable(sct_target_fifo sc_main.cpp)
target_compile_definitions(sct_target_fifo PUBLIC -DTARGET_FIFO -DMETHOD)
svc_target(sct_target_fifo ELAB_TOP test_top.dut GOLDEN sct_target_fifo.sv)
