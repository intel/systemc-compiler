#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Example latch and FF 
project(latch_ff)

# All synthesizable source files must be listed here (not in libraries)
add_executable(latch_ff example.cpp)

# Test source directory
target_include_directories(latch_ff PUBLIC 
    $ENV{ICSC_HOME}/examples/latch_ff
)

# Add compilation options
# target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

# Add optional library, no add SystemC library (it added by svc_target)
#target_link_libraries(${PROJECT_NAME} sometestbenchlibrary)

# svc_target will create ${PROJECT_NAME}_sctool executable that runs code generation 
# and ${PROJECT_NAME} that runs general SystemC simulation
# ELAB_TOP parameter accepts hierarchical name of design  
# (that is SystemC name, returned by sc_object::name() method)
svc_target(latch_ff ELAB_TOP tb.dut_inst)
