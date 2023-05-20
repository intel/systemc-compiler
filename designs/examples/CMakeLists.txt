#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

cmake_minimum_required(VERSION 3.12)

enable_testing()

if(NOT DEFINED ENV{ICSC_HOME})
  message("ICSC_HOME is not defined!")
  return()
endif()

project(icsc_examples)

## SVC package contains ScTool and SystemC libraries
find_package(SVC REQUIRED)

# C++ standard must be the same as in ScTool, $(SystemC_CXX_STANDARD) contains 17
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(asserts)
add_subdirectory(counter)
add_subdirectory(decoder)
add_subdirectory(dma)
add_subdirectory(dm_paranut)
add_subdirectory(dvcon20)
add_subdirectory(fsm)
add_subdirectory(intrinsic)
add_subdirectory(latch_ff)

add_subdirectory(sct_fifo)
add_subdirectory(sct_initiator)
add_subdirectory(sct_target)
add_subdirectory(sct_signal)
add_subdirectory(sct_always_ready)
