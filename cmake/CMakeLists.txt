#******************************************************************************
# Copyright (c) 2026, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************
#
# Intel(r) Compiler for SystemC*
#
# *****************************************************************************
 
cmake_minimum_required(VERSION 3.12)
project (sc_tools_designs)

## C++ standard 
set(CMAKE_CXX_STANDARD 20)

enable_testing()

## SVC package contains ICSC and SystemC libraries
find_package(SVC REQUIRED)

## Add examples, tests and user designs
add_subdirectory(designs/examples)
add_subdirectory(designs/tests)
add_subdirectory(designs/single_source)
add_subdirectory(designs/template)