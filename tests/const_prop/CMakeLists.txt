#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(const_negative test_const_negative.cpp)
svc_target(const_negative GOLDEN const_negative.sv)

add_executable(test_const_prop_basic test_const_prop_basic.cpp)
svc_target(test_const_prop_basic)

add_executable(test_const_prop_switch test_const_prop_switch.cpp)
svc_target(test_const_prop_switch )

add_executable(test_const_prop_dowhile test_const_prop_dowhile.cpp)
svc_target(test_const_prop_dowhile )

add_executable(test_const_prop_fork_return test_const_prop_fork_return.cpp)
svc_target(test_const_prop_fork_return )

add_executable(test_const_prop_loops test_const_prop_loops.cpp)
svc_target(test_const_prop_loops )

add_executable(test_const_prop_deep_fork test_const_prop_deep_fork.cpp)
svc_target(test_const_prop_deep_fork )

add_executable(test_const_prop_logic_term test_const_prop_logic_term.cpp)
svc_target(test_const_prop_logic_term )

add_executable(test_const_prop_stop_wait test_const_prop_stop_wait.cpp)
svc_target(test_const_prop_stop_wait )

add_executable(test_const_prop_deep_wait test_const_prop_deep_wait.cpp)
svc_target(test_const_prop_deep_wait )

add_executable(test_const_prop_cthread test_const_prop_cthread.cpp)
svc_target(test_const_prop_cthread )

add_executable(test_const_prop_fcall_simple test_const_prop_fcall_simple.cpp)
svc_target(test_const_prop_fcall_simple )

add_executable(test_const_prop_virtual_calls test_const_prop_virtual_calls.cpp)
svc_target(test_const_prop_virtual_calls )

add_executable(test_const_prop_multiple_inheritance test_const_prop_multiple_inheritance.cpp)
svc_target(test_const_prop_multiple_inheritance )

add_executable(test_const_prop_virtual_inheritance test_const_prop_virtual_inheritance.cpp)
svc_target(test_const_prop_virtual_inheritance )

add_executable(test_const_prop_fork_continue test_const_prop_fork_continue.cpp)
svc_target(test_const_prop_fork_continue )

add_executable(test_const_prop_fork_inner_loop test_const_prop_fork_inner_loop.cpp)
svc_target(test_const_prop_fork_inner_loop )

add_executable(test_const_prop_inner_loop_wait test_const_prop_inner_loop_wait.cpp)
svc_target(test_const_prop_inner_loop_wait )

add_executable(test_const_prop_dead_state test_const_prop_dead_state.cpp)
svc_target(test_const_prop_dead_state )

add_executable(test_const_prop_cthread_wait_return test_const_prop_cthread_wait_return.cpp)
svc_target(test_const_prop_cthread_wait_return )

add_executable(test_const_prop_eval_stmts test_const_prop_eval_stmts.cpp)
svc_target(test_const_prop_eval_stmts )

add_executable(test_const_prop_float test_const_prop_float.cpp)
svc_target(test_const_prop_float  WILL_FAIL)

add_executable(test_const_prop_sc_dt test_const_prop_sc_dt.cpp)
svc_target(test_const_prop_sc_dt )

add_executable(test_const_prop_unsupported_sc_dt test_const_prop_sc_dt_unsupported.cpp)
svc_target(test_const_prop_unsupported_sc_dt  WILL_FAIL)

add_executable(test_const_prop_many_forks test_const_prop_many_forks.cpp)
svc_target(test_const_prop_many_forks )


