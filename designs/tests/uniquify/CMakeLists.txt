#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(test_uniquify_basic test_uniquify_basic.cpp)
svc_target(test_uniquify_basic GOLDEN test_uniquify_basic.sv)

add_executable(test_uniquify_proc test_uniquify_proc.cpp)
svc_target(test_uniquify_proc GOLDEN test_uniquify_proc.sv)

add_executable(test_uniquify_cross_bind test_uniquify_cross_bind.cpp)
svc_target(test_uniquify_cross_bind GOLDEN test_uniquify_cross_bind.sv)
