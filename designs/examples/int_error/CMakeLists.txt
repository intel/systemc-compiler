#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Example counter
project(int_error)

# All synthesizable source files must be listed here (not in libraries)
add_executable(int_error example.cpp)

# svc_target will create @example_sctool executable that runs code generation 
# and @example that runs general SystemC simulation
# ELAB_TOP parameter accepts hierarchical name of DUT  
# (that is SystemC name, returned by sc_object::name() method)
svc_target(int_error ELAB_TOP dut WILL_FAIL)