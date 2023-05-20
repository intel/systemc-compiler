#******************************************************************************
# Copyright (c) 2023, Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
#
# *****************************************************************************

#
# sct_property in thread property test
#
if (RTL_MODE_TESTS)
    add_executable(sct_assert_test-rtl sc_main.cpp)
    #target_compile_definitions(sct_assert_test-rtl PUBLIC )
    add_test(NAME sct_assert_test-rtl COMMAND sct_assert_test-rtl)
    
    add_executable(sct_assert_chan_test-rtl sc_main2.cpp)
    #target_compile_definitions(sct_assert_chan_test-rtl PUBLIC )
    add_test(NAME sct_assert_chan_test-rtl COMMAND sct_assert_chan_test-rtl)
endif()

if (TLM_MODE_TESTS)
    add_executable(sct_assert_test-tlm sc_main.cpp)
    target_compile_definitions(sct_assert_test-tlm PUBLIC SCT_TLM_MODE)
    add_test(NAME sct_assert_test-tlm COMMAND sct_assert_test-tlm)
    
    add_executable(sct_assert_chan_test-tlm sc_main2.cpp)
    target_compile_definitions(sct_assert_chan_test-tlm PUBLIC SCT_TLM_MODE)
    add_test(NAME sct_assert_chan_test-tlm COMMAND sct_assert_chan_test-tlm)
endif()
