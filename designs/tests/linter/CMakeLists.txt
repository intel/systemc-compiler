#******************************************************************************
# Copyright (c) 2026, Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
#
# *****************************************************************************

add_executable(linter_overflow1 linter_overflow1.cpp)
svc_target(linter_overflow1 ELAB_TOP tb.dut)

add_executable(linter_synchronize1 linter_synchronize1.cpp)
svc_target(linter_synchronize1 ELAB_TOP tb.dut)
