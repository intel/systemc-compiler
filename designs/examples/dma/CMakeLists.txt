# ******************************************************************************
# * Copyright (c) 2020, Intel Corporation. All rights reserved.
# * 
# * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# * 
# *****************************************************************************/



cmake_minimum_required(VERSION 3.12)

# Enable ctest command
enable_testing()

project(DmaEngine_NoSs)

## SVC package contains ScTool and SystemC libraries
find_package(SVC REQUIRED)

set(PROC_TECH fpga)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# All synthesizable source files must be listed here (not in libraries)
add_executable(${PROJECT_NAME} GenDma_tb.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE PROC_TECH_${PROC_TECH})
#target_compile_definitions(${PROJECT_NAME} PUBLIC -DSCT_TLM_MODE)
target_compile_options(${PROJECT_NAME} PUBLIC "-Ofast")
svc_target(${PROJECT_NAME} INIT_LOCAL_VARS ELAB_TOP tb.dut)
#add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
