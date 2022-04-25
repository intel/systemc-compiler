#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

## Arrays
add_executable(cthread_array test_cthread_array.cpp)
svc_target(cthread_array GOLDEN cthread_array.sv)

add_executable(cthread_array_access test_cthread_array_access.cpp)
svc_target(cthread_array_access GOLDEN cthread_array_access.sv)

add_executable(cthread_array_comb_reg test_cthread_array_comb_reg.cpp)
svc_target(cthread_array_comb_reg GOLDEN cthread_array_comb_reg.sv)


add_executable(cthread_simple test_cthread_simple.cpp)
svc_target(cthread_simple GOLDEN cthread_simple.sv)

add_executable(cthread_simple2 test_cthread_simple2.cpp)
svc_target(cthread_simple2 GOLDEN cthread_simple2.sv)

# Reset related tests
add_executable(cthread_reset test_cthread_reset.cpp)
svc_target(cthread_reset GOLDEN cthread_reset.sv)

add_executable(cthread_reset_warning test_cthread_reset_warning.cpp)
svc_target(cthread_reset_warning GOLDEN cthread_reset_warning.sv)

add_executable(cthread_reset_fail test_cthread_reset_fail.cpp)
svc_target(cthread_reset_fail GOLDEN cthread_reset_fail.sv
           WILL_FAIL)

add_executable(cthread_reset_noninit_read test_reset_noninit_read.cpp)
svc_target(cthread_reset_noninit_read GOLDEN cthread_reset_noninit_read.sv)

add_executable(cthread_reset_after test_cthread_reset_after.cpp)
svc_target(cthread_reset_after GOLDEN cthread_reset_after.sv)

add_executable(cthread_simple_reset test_cthread_simple_reset.cpp)
svc_target(cthread_simple_reset GOLDEN cthread_simple_reset.sv)

add_executable(cthread_mult_resets test_cthread_mult_resets.cpp)
svc_target(cthread_mult_resets GOLDEN cthread_mult_resets.sv)

add_executable(cthread_multi_wait_reset test_cthread_multi_wait_reset.cpp)
svc_target(cthread_multi_wait_reset GOLDEN cthread_multi_wait_reset.sv)

add_executable(cthread_sync_async_reset test_cthread_sync_async_reset.cpp)
svc_target(cthread_sync_async_reset GOLDEN cthread_sync_async_reset.sv)

add_executable(cthread_fcall_in_reset test_fcall_in_reset.cpp)
svc_target(cthread_fcall_in_reset GOLDEN cthread_fcall_in_reset.sv)

add_executable(cthread_no_reset_fail test_no_reset_fail.cpp)
svc_target(cthread_no_reset_fail WILL_FAIL)

# Loops
add_executable(cthread_for test_cthread_for.cpp)
svc_target(cthread_for GOLDEN cthread_for.sv)

add_executable(cthread_fcall_loop_fail test_fcall_loop_fail.cpp)
svc_target(cthread_fcall_loop_fail WILL_FAIL)

add_executable(cthread_for_other_types test_cthread_for_other_types.cpp)
svc_target(cthread_for_other_types GOLDEN cthread_for_other_types.sv)

add_executable(cthread_while test_while.cpp)
svc_target(cthread_while GOLDEN cthread_while.sv)

add_executable(cthread_while_other_types test_while_other_types.cpp)
svc_target(cthread_while_other_types GOLDEN cthread_while_other_types.sv)

add_executable(cthread_while_const test_while_const.cpp)
svc_target(cthread_while_const GOLDEN cthread_while_const.sv)

add_executable(cthread_do_while test_do_while.cpp)
svc_target(cthread_do_while GOLDEN cthread_do_while.sv)

add_executable(cthread_do_while_other_types test_do_while_other_types.cpp)
svc_target(cthread_do_while_other_types GOLDEN cthread_do_while_other_types.sv)

add_executable(cthread_do_while_always1 test_do_while_always1.cpp)
svc_target(cthread_do_while_always1 GOLDEN cthread_do_while_always1.sv)
           
add_executable(cthread_do_while_const test_do_while_const.cpp)
svc_target(cthread_do_while_const GOLDEN cthread_do_while_const.sv)

add_executable(cthread_inner_loops test_cthread_inner_loops.cpp)
svc_target(cthread_inner_loops GOLDEN cthread_inner_loops.sv)

add_executable(cthread_alive_loop test_cthread_alive_loop.cpp)
svc_target(cthread_alive_loop GOLDEN cthread_alive_loop.sv)

add_executable(cthread_loop_fail test_cthread_loop_fail.cpp)
svc_target(cthread_loop_fail WILL_FAIL)

add_executable(cthread_loop_fail2 test_cthread_loop_fail2.cpp)
svc_target(cthread_loop_fail2 WILL_FAIL)

add_executable(cthread_loop_fail3 test_cthread_loop_fail3.cpp)
svc_target(cthread_loop_fail3 WILL_FAIL)

add_executable(cthread_loop_fail4 test_cthread_loop_fail4.cpp)
svc_target(cthread_loop_fail4 WILL_FAIL)

add_executable(cthread_loop_fail5 test_cthread_loop_fail5.cpp)
svc_target(cthread_loop_fail5 WILL_FAIL)

add_executable(cthread_loop_fail6 test_cthread_loop_fail6.cpp)
svc_target(cthread_loop_fail6 WILL_FAIL)

add_executable(cthread_loop_fail7 test_cthread_loop_fail7.cpp)
svc_target(cthread_loop_fail7 WILL_FAIL)

## Control flow
add_executable(cthread_if test_if.cpp)
svc_target(cthread_if GOLDEN cthread_if.sv)

add_executable(cthread_if_const test_if_const.cpp)
svc_target(cthread_if_const GOLDEN cthread_if_const.sv)

add_executable(cthread_break test_break.cpp)
svc_target(cthread_break GOLDEN cthread_break.sv)

add_executable(cthread_continue test_continue.cpp)
svc_target(cthread_continue GOLDEN cthread_continue.sv)

## Function calls
add_executable(cthread_fcall test_cthread_fcall.cpp)
svc_target(cthread_fcall GOLDEN cthread_fcall.sv)

add_executable(cthread_fcall_ref test_fcall_ref.cpp)
svc_target(cthread_fcall_ref GOLDEN cthread_fcall_ref.sv)

add_executable(cthread_fcall_const_eval test_cthread_fcall_const_eval.cpp)
svc_target(cthread_fcall_const_eval GOLDEN cthread_fcall_const_eval.sv)

add_executable(cthread_return test_return.cpp)
svc_target(cthread_return GOLDEN cthread_return.sv)

add_executable(cthread_switch test_cthread_switch.cpp)
svc_target(cthread_switch GOLDEN cthread_switch.sv)

add_executable(cthread_switch_inside test_cthread_switch_inside.cpp)
svc_target(cthread_switch_inside GOLDEN cthread_switch_inside.sv)

add_executable(cthread_switch_after test_cthread_switch_after.cpp)
svc_target(cthread_switch_after GOLDEN cthread_switch_after.sv)

add_executable(cthread_switch_default test_cthread_switch_default.cpp)
svc_target(cthread_switch_default GOLDEN cthread_switch_default.sv)

add_executable(cthread_switch_empty_case test_cthread_switch_empty_case.cpp)
svc_target(cthread_switch_empty_case GOLDEN cthread_switch_empty_case.sv)

add_executable(cthread_for_cntr_scint test_cthread_for_cntr_scint.cpp)
svc_target(cthread_for_cntr_scint GOLDEN cthread_for_cntr_scint.sv)

add_executable(cthread_break_via_func test_cthread_break_via_func.cpp)
svc_target(cthread_break_via_func GOLDEN cthread_break_via_func.sv)

add_executable(cthread_for_if test_cthread_for_if.cpp)
svc_target(cthread_for_if GOLDEN cthread_for_if.sv)

add_executable(cthread_virtual_wait test_cthread_virtual_wait.cpp)
svc_target(cthread_virtual_wait GOLDEN cthread_virtual_wait.sv)

add_executable(cthread_wait_n test_cthread_wait_n.cpp)
svc_target(cthread_wait_n GOLDEN cthread_wait_n.sv)

add_executable(cthread_wait_n_fail test_cthread_wait_n_fail.cpp)
svc_target(cthread_wait_n_fail WILL_FAIL)

add_executable(cthread_wait_n_fail2 test_cthread_wait_n_fail2.cpp)
svc_target(cthread_wait_n_fail2 WILL_FAIL)

add_executable(cthread_duplicate_states test_cthread_duplicate_states.cpp)
svc_target(cthread_duplicate_states GOLDEN cthread_duplicate_states.sv)

add_executable(cthread_read_defined_unknown test_cthread_read_defined_unknown.cpp)
svc_target(cthread_read_defined_unknown GOLDEN cthread_read_defined_unknown.sv)

add_executable(cthread_read_defined test_cthread_read_defined.cpp)
svc_target(cthread_read_defined GOLDEN cthread_read_defined.sv)

add_executable(cthread_read_defined_0 test_cthread_read_defined_0.cpp)
svc_target(cthread_read_defined_0 GOLDEN cthread_read_defined_0.sv)

add_executable(cthread_read_defined_1 test_cthread_read_defined_1.cpp)
svc_target(cthread_read_defined_1 GOLDEN cthread_read_defined_1.sv)

add_executable(cthread_read_defined_2 test_cthread_read_defined_2.cpp)
svc_target(cthread_read_defined_2 GOLDEN cthread_read_defined_2.sv)

add_executable(cthread_read_defined_3 test_cthread_read_defined_3.cpp)
svc_target(cthread_read_defined_3 GOLDEN cthread_read_defined_3.sv)

add_executable(cthread_read_defined_param1 test_cthread_read_defined_param1.cpp)
svc_target(cthread_read_defined_param1 GOLDEN cthread_read_defined_param1.sv)

add_executable(cthread_read_defined_param2 test_cthread_read_defined_param2.cpp)
svc_target(cthread_read_defined_param2 GOLDEN cthread_read_defined_param2.sv)

add_executable(cthread_read_defined_param3 test_cthread_read_defined_param3.cpp)
svc_target(cthread_read_defined_param3 GOLDEN cthread_read_defined_param3.sv)

add_executable(cthread_read_defined_param4 test_cthread_read_defined_param4.cpp)
svc_target(cthread_read_defined_param4 GOLDEN cthread_read_defined_param4.sv)

add_executable(cthread_read_defined_ref test_cthread_read_defined_ref.cpp)
svc_target(cthread_read_defined_ref GOLDEN cthread_read_defined_ref.sv)

add_executable(cthread_wait_states test_cthread_wait_states.cpp)
svc_target(cthread_wait_states GOLDEN cthread_wait_states.sv)

add_executable(cthread_read_only test_cthread_read_only.cpp)
svc_target(cthread_read_only GOLDEN cthread_read_only.sv)

## Constants
add_executable(cthread_const_array test_cthread_const_array.cpp)
svc_target(cthread_const_array GOLDEN cthread_const_array.sv)

add_executable(cthread_const_fail test_cthread_const_fail.cpp)
svc_target(cthread_const_fail WILL_FAIL)

add_executable(cthread_const_prop test_cthread_const_prop.cpp)
svc_target(cthread_const_prop GOLDEN cthread_const_prop.sv)

add_executable(cthread_mif_array_differ test_mif_array_differ.cpp)
svc_target(cthread_mif_array_differ GOLDEN cthread_mif_array_differ.sv)

add_executable(cthread_mod_array_differ test_mod_array_differ.cpp)
svc_target(cthread_mod_array_differ GOLDEN cthread_mod_array_differ.sv)

add_executable(cthread_varname test_cthread_varname.cpp)
svc_target(cthread_varname GOLDEN cthread_varname.sv)

add_executable(cthread_sel_bit test_cthread_sel_bit.cpp)
svc_target(cthread_sel_bit GOLDEN cthread_sel_bit.sv)

add_executable(cthread_sel_range test_cthread_sel_range.cpp)
svc_target(cthread_sel_range GOLDEN cthread_sel_range.sv)

## Pointers
add_executable(cthread_pointers test_pointers.cpp) 
svc_target(cthread_pointers GOLDEN cthread_pointers.sv)

add_executable(cthread_pointer_to_bool test_pointer_to_bool.cpp)
svc_target(cthread_pointer_to_bool GOLDEN cthread_pointer_to_bool.sv)


add_executable(cthread_concat test_cthread_concat.cpp)
svc_target(cthread_concat GOLDEN cthread_concat.sv)

add_executable(cthread_initvar_inf_loop test_cthread_initvar_inf_loop.cpp)
svc_target(cthread_initvar_inf_loop GOLDEN cthread_initvar_inf_loop.sv)


add_executable(rtl_fifo test_rtl_fifo.cpp)
svc_target(rtl_fifo GOLDEN rtl_fifo.sv)

add_executable(bhv_fifo test_bhv_fifo.cpp)
svc_target(bhv_fifo GOLDEN bhv_fifo.sv)
           
## Variables
add_executable(cthread_var_reg test_cthread_var_reg.cpp)
svc_target(cthread_var_reg GOLDEN cthread_var_reg.sv)

add_executable(cthread_var_comb test_cthread_var_comb.cpp)
svc_target(cthread_var_comb GOLDEN cthread_var_comb.sv)

add_executable(cthread_var_const test_cthread_var_const.cpp)
svc_target(cthread_var_const GOLDEN cthread_var_const.sv)

add_executable(cthread_var_ref test_cthread_var_ref.cpp)
svc_target(cthread_var_ref GOLDEN cthread_var_ref.sv)

add_executable(cthread_var_multiple_use test_cthread_var_multiple_use.cpp)
svc_target(cthread_var_multiple_use GOLDEN cthread_var_multiple_use.sv)


