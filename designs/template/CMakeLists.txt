#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

cmake_minimum_required(VERSION 3.12)

enable_testing()

# Design template
project(mydesign)

## SVC package contains ScTool and SystemC libraries
# The SVC package will set CMAKE_CXX_STANDARD and CMAKE_CXX_STANDARD_REQUIRED
find_package(SVC REQUIRED)

# All synthesizable source files must be listed here (not in libraries)
add_executable(mydesign example.cpp)

# Add compilation options
# target_compile_definitions(mydesign PUBLIC -DMYOPTION)
# target_compile_options(mydesign PUBLIC -Wall)

# Add optional library, do not add SystemC library (it added by svc_target)
#target_link_libraries(mydesign sometestbenchlibrary)

# svc_target will create @mydesign_sctool executable that runs code generation 
# and @mydesign that runs general SystemC simulation
# ELAB_TOP parameter accepts hierarchical name of DUT  
# (that is SystemC name, returned by sc_object::name() method)
svc_target(mydesign INIT_LOCAL_VARS ELAB_TOP tb.dut_inst)
