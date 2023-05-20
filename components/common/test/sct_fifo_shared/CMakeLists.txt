#******************************************************************************
# Copyright (c) 2023, Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
#
# *****************************************************************************

#
# Single source library testbench 
#

if (RTL_MODE_TESTS)
    add_executable(sct_fifo_shared_meth_sig-rtl sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_meth_sig-rtl PUBLIC METHOD_SIG)
    add_test(NAME sct_fifo_shared_meth_sig-rtl COMMAND sct_fifo_shared_meth_sig-rtl)

    add_executable(sct_fifo_shared_meth-rtl sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_meth-rtl PUBLIC METHOD)
    add_test(NAME sct_fifo_shared_meth-rtl COMMAND sct_fifo_shared_meth-rtl)

    add_executable(sct_fifo_shared_thread-rtl sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_thread-rtl PUBLIC THREAD)
    add_test(NAME sct_fifo_shared_thread-rtl COMMAND sct_fifo_shared_thread-rtl)

    # Check minimal size to provide full throughput
    add_executable(sct_fifo_shared_size-rtl sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_size-rtl PUBLIC FSIZE)
    add_test(NAME sct_fifo_shared_size-rtl COMMAND sct_fifo_shared_size-rtl)

    # Multiple sequential put/get
    add_executable(sct_fifo_shared_multu_put-rtl sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_multu_put-rtl PUBLIC MULTI_PUT)
    add_test(NAME sct_fifo_shared_multu_put-rtl COMMAND sct_fifo_shared_multu_put-rtl)
endif()

if (TLM_MODE_TESTS)
    add_executable(sct_fifo_shared_meth_sig-tlm sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_meth_sig-tlm PUBLIC METHOD_SIG SCT_TLM_MODE)
    add_test(NAME sct_fifo_shared_meth_sig-tlm COMMAND sct_fifo_shared_meth_sig-tlm)

    add_executable(sct_fifo_shared_meth-tlm sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_meth-tlm PUBLIC METHOD SCT_TLM_MODE)
    add_test(NAME sct_fifo_shared_meth-tlm COMMAND sct_fifo_shared_meth-tlm)

    add_executable(sct_fifo_shared_thread-tlm sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_thread-tlm PUBLIC THREAD SCT_TLM_MODE)
    add_test(NAME sct_fifo_shared_thread-tlm COMMAND sct_fifo_shared_thread-tlm)

    # Check minimal size to provide full throughput
    add_executable(sct_fifo_shared_size-tlm sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_size-tlm PUBLIC FSIZE SCT_TLM_MODE)
    add_test(NAME sct_fifo_shared_size-tlm COMMAND sct_fifo_shared_size-tlm)

    # Multiple sequential put/get
    add_executable(sct_fifo_shared_multu_put-tlm sc_main.cpp)
    target_compile_definitions(sct_fifo_shared_multu_put-tlm PUBLIC MULTI_PUT SCT_TLM_MODE)
    add_test(NAME sct_fifo_shared_multu_put-tlm COMMAND sct_fifo_shared_multu_put-tlm)
endif()
