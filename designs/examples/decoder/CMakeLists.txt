#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Example encoder and decoder
project(decoder)

# All synthesizable source files must be listed here (not in libraries)
add_executable(decoder example.cpp)

# Test source directory
target_include_directories(decoder PUBLIC designs/examples/decoder)

# svc_target will create ${PROJECT_NAME}_sctool executable that runs code generation 
# and ${PROJECT_NAME} that runs general SystemC simulation
# ELAB_TOP parameter accepts hierarchical name of design  
# (that is SystemC name, returned by sc_object::name() method)
svc_target(decoder ELAB_TOP tb.dut_inst)
