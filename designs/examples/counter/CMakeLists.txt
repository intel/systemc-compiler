#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Example counter
project(counter)

# All synthesizable source files must be listed here (not in libraries)
add_executable(counter example.cpp)

svc_target(counter ELAB_TOP tb.dut_inst)