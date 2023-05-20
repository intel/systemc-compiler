#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(sct_signal sc_main.cpp)
target_compile_definitions(sct_signal PUBLIC -DSIGNAL)
svc_target(sct_signal ELAB_TOP test_top.dut GOLDEN sct_signal.sv)

add_executable(sct_in_out_ports sc_main.cpp)
target_compile_definitions(sct_in_out_ports PUBLIC -DPORT)
svc_target(sct_in_out_ports ELAB_TOP test_top.dut GOLDEN sct_in_out_ports.sv)
