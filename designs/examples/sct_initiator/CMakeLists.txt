#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

## Initiator used in THREAD
add_executable(sct_initiator_thread sc_main.cpp)
svc_target(sct_initiator_thread ELAB_TOP test_top.dut GOLDEN sct_initiator_thread.sv)
