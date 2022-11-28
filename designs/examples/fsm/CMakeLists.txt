#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Example FSM implementation
project(fsm)

# All synthesizable source files must be listed here (not in libraries)
add_executable(fsm example.cpp)

# Test source directory
target_include_directories(fsm PUBLIC 
    $ENV{ICSC_HOME}/examples/fsm
)

# Add compilation options
# target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

# Add optional library, no add SystemC library (it added by svc_target)
#target_link_libraries(${PROJECT_NAME} sometestbenchlibrary)

# svc_target will create ${PROJECT_NAME}_sctool executable that runs code generation 
# and ${PROJECT_NAME} that runs general SystemC simulation
# ELAB_TOP parameter accepts hierarchical name of design  
# (that is SystemC name, returned by sc_object::name() method)
svc_target(fsm ELAB_TOP tb.dut_inst)
