#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# CMake for copilot_apb_port
add_executable(copilot_apb_port
    sc_main.cpp
)

# Test source directory
target_include_directories(copilot_apb_port PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Mark top for elaboration (assuming svc_target macro available)
svc_target(copilot_apb_port ELAB_TOP tbt.dut)
