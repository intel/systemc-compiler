#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

cmake_minimum_required(VERSION 3.12)

enable_testing()

project(icsc_tests)

## SVC package contains ScTool and SystemC libraries
find_package(SVC REQUIRED)

add_subdirectory(const_prop)
add_subdirectory(cthread)
add_subdirectory(method)
add_subdirectory(mif)
add_subdirectory(misc)
add_subdirectory(record)
add_subdirectory(state)
add_subdirectory(uniquify)

