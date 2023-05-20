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
    add_executable(sct_simple_method_sig-rtl sc_main.cpp)
    target_compile_definitions(sct_simple_method_sig-rtl PUBLIC METHOD_SIG)
    add_test(NAME sct_simple_method_sig-rtl COMMAND sct_simple_method_sig-rtl)

    add_executable(sct_simple_method-rtl sc_main.cpp)
    target_compile_definitions(sct_simple_method-rtl PUBLIC METHOD)
    add_test(NAME sct_simple_method-rtl COMMAND sct_simple_method-rtl)

    add_executable(sct_multi_put_thread-rtl sc_main.cpp)
    target_compile_definitions(sct_multi_put_thread-rtl PUBLIC MULTI_PUT)
    add_test(NAME sct_multi_put_thread-rtl COMMAND sct_multi_put_thread-rtl)

    add_executable(sct_simple_thread-rtl sc_main.cpp)
    target_compile_definitions(sct_simple_thread-rtl PUBLIC THREAD)
    add_test(NAME sct_simple_thread-rtl COMMAND sct_simple_thread-rtl)

    add_executable(sct_simple_bput-rtl sc_main.cpp)
    target_compile_definitions(sct_simple_bput-rtl PUBLIC BPUT)
    add_test(NAME sct_simple_bput-rtl COMMAND sct_simple_bput-rtl)

    add_executable(sct_multi_state_thread-rtl sc_main.cpp)
    target_compile_definitions(sct_multi_state_thread-rtl PUBLIC MULTI_STATE)
    add_test(NAME sct_multi_state_thread-rtl COMMAND sct_multi_state_thread-rtl)

    add_executable(sct_clkrst_traits-rtl sc_main.cpp)
    target_compile_definitions(sct_clkrst_traits-rtl PUBLIC CLK_RST)
    add_test(NAME sct_clkrst_traits-rtl COMMAND sct_clkrst_traits-rtl)

    add_executable(sct_def_traits-rtl sc_main.cpp)
    target_compile_definitions(sct_def_traits-rtl PUBLIC DEF SCT_TLM_MODE)
    target_compile_definitions(sct_def_traits-rtl PUBLIC SCT_CMN_TRAITS=SCT_NEGEDGE_POSRESET)
    add_test(NAME sct_def_traits-rtl COMMAND sct_def_traits-rtl)

    add_executable(sct_targ_vect-rtl sc_main.cpp)
    target_compile_definitions(sct_targ_vect-rtl PUBLIC TARG_VECT)
    add_test(NAME sct_targ_vect-rtl COMMAND sct_targ_vect-rtl)

    add_executable(sct_bind_testbench-rtl sc_main2.cpp)
    #target_compile_definitions(sct_bind_testbench-rtl PUBLIC )
    add_test(NAME sct_bind_testbench-rtl COMMAND sct_bind_testbench-rtl)
endif()

if (TLM_MODE_TESTS)
    add_executable(sct_simple_method_sig-tlm sc_main.cpp)
    target_compile_definitions(sct_simple_method_sig-tlm PUBLIC METHOD_SIG SCT_TLM_MODE)
    add_test(NAME sct_simple_method_sig-tlm COMMAND sct_simple_method_sig-tlm)

    add_executable(sct_simple_method-tlm sc_main.cpp)
    target_compile_definitions(sct_simple_method-tlm PUBLIC METHOD SCT_TLM_MODE)
    add_test(NAME sct_simple_method-tlm COMMAND sct_simple_method-tlm)

    add_executable(sct_multi_put_thread-tlm sc_main.cpp)
    target_compile_definitions(sct_multi_put_thread-tlm PUBLIC MULTI_PUT SCT_TLM_MODE)
    add_test(NAME sct_multi_put_thread-tlm COMMAND sct_multi_put_thread-tlm)

    add_executable(sct_simple_thread-tlm sc_main.cpp)
    target_compile_definitions(sct_simple_thread-tlm PUBLIC THREAD SCT_TLM_MODE)
    add_test(NAME sct_simple_thread-tlm COMMAND sct_simple_thread-tlm)

    add_executable(sct_simple_bput-tlm sc_main.cpp)
    target_compile_definitions(sct_simple_bput-tlm PUBLIC BPUT SCT_TLM_MODE)
    add_test(NAME sct_simple_bput-tlm COMMAND sct_simple_bput-tlm)

    add_executable(sct_multi_state_thread-tlm sc_main.cpp)
    target_compile_definitions(sct_multi_state_thread-tlm PUBLIC MULTI_STATE SCT_TLM_MODE)
    add_test(NAME sct_multi_state_thread-tlm COMMAND sct_multi_state_thread-tlm)

    add_executable(sct_clkrst_traits-tlm sc_main.cpp)
    target_compile_definitions(sct_clkrst_traits-tlm PUBLIC CLK_RST SCT_TLM_MODE)
    add_test(NAME sct_clkrst_traits-tlm COMMAND sct_clkrst_traits-tlm)

    add_executable(sct_def_traits-tlm sc_main.cpp)
    target_compile_definitions(sct_def_traits-tlm PUBLIC DEF SCT_TLM_MODE SCT_TLM_MODE)
    target_compile_definitions(sct_def_traits-tlm PUBLIC SCT_CMN_TRAITS=SCT_NEGEDGE_POSRESET)
    add_test(NAME sct_def_traits-tlm COMMAND sct_def_traits-tlm)

    add_executable(sct_targ_vect-tlm sc_main.cpp)
    target_compile_definitions(sct_targ_vect-tlm PUBLIC TARG_VECT SCT_TLM_MODE)
    add_test(NAME sct_targ_vect-tlm COMMAND sct_targ_vect-tlm)
endif()
