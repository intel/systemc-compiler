#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

#######################################################################
# HTR NEW TESTS
add_executable(method_stdouts_prints test_stdouts_prints.cpp)
svc_target(method_stdouts_prints GOLDEN method_stdouts_prints.sv)

add_executable(method_trace test_trace.cpp)
svc_target(method_trace)

#######################################################################

add_executable(method_level1 test_level1.cpp)
svc_target(method_level1 GOLDEN method_level1.sv)

add_executable(method_level2 test_level2.cpp)
svc_target(method_level2 GOLDEN method_level2.sv)

add_executable(method_level_error test_level_error.cpp)
svc_target(method_level_error GOLDEN method_level_error.sv)

add_executable(method_break test_break.cpp)
svc_target(method_break GOLDEN method_break.sv)

add_executable(method_continue test_continue.cpp)
svc_target(method_continue GOLDEN method_continue.sv)

add_executable(method_while test_while.cpp)
svc_target(method_while GOLDEN method_while.sv)

add_executable(method_while_other test_while_other.cpp)
svc_target(method_while_other GOLDEN method_while_other.sv)

add_executable(method_while_const test_while_const.cpp)
svc_target(method_while_const GOLDEN method_while_const.sv)

add_executable(method_dowhile test_dowhile.cpp)
svc_target(method_dowhile GOLDEN method_dowhile.sv)

add_executable(method_dowhile_other test_dowhile_other.cpp)
svc_target(method_dowhile_other GOLDEN method_dowhile_other.sv)

add_executable(method_for test_for.cpp)
svc_target(method_for GOLDEN method_for.sv)

add_executable(method_for_fcall_fail test_for_fcall_fail.cpp)
svc_target(method_for_fcall_fail WILL_FAIL)

add_executable(method_while_fcall_fail test_while_fcall_fail.cpp)
svc_target(method_while_fcall_fail WILL_FAIL)


## Constants
add_executable(method_const test_const.cpp)
svc_target(method_const GOLDEN method_const.sv)

add_executable(method_const_liter test_const_liter.cpp)
svc_target(method_const_liter GOLDEN method_const_liter.sv)

add_executable(method_const_liter_compound test_const_liter_compound.cpp)
svc_target(method_const_liter_compound GOLDEN method_const_liter_compound.sv)

add_executable(method_string_liter test_string_liter.cpp)
svc_target(method_string_liter GOLDEN method_string_liter.sv)

add_executable(method_liter_width test_liter_width.cpp)
svc_target(method_liter_width GOLDEN method_liter_width.sv)

add_executable(method_const_binary test_const_binary.cpp)
svc_target(method_const_binary GOLDEN method_const_binary.sv)

add_executable(method_const_if test_const_if.cpp)
svc_target(method_const_if GOLDEN method_const_if.sv)

add_executable(method_const_if2 test_const_if2.cpp)
svc_target(method_const_if2 GOLDEN method_const_if2.sv)

add_executable(method_const_fail test_const_fail.cpp)
svc_target(method_const_fail WILL_FAIL)

add_executable(method_const_static_mem test_const_static_mem.cpp)
svc_target(method_const_static_mem GOLDEN method_const_static_mem.sv)

## Uncomment me after #223
#add_executable(method_sc_int_const_array test_sc_int_const_array.cpp)
#svc_target(method_sc_int_const_array)

# Function call
add_executable(method_fcall test_fcall.cpp)
svc_target(method_fcall GOLDEN method_fcall.sv)

add_executable(method_fcall_base test_fcall_base.cpp)
svc_target(method_fcall_base GOLDEN method_fcall_base.sv)

add_executable(method_generic_fcall test_generic_fcall.cpp)
svc_target(method_generic_fcall GOLDEN method_generic_fcall.sv)

add_executable(method_fcall_const_eval test_fcall_const_eval.cpp)
svc_target(method_fcall_const_eval GOLDEN method_fcall_const_eval.sv)

add_executable(method_fcall_const_eval2 test_fcall_const_eval2.cpp)
svc_target(method_fcall_const_eval2 GOLDEN method_fcall_const_eval2.sv)

add_executable(method_return test_return.cpp)
svc_target(method_return GOLDEN method_return.sv)

add_executable(method_return_fail test_return_fail.cpp)
svc_target(method_return_fail WILL_FAIL)


add_executable(method_virtual1 test_virtual1.cpp)
svc_target(method_virtual1 GOLDEN method_virtual1.sv)

add_executable(method_virtual2 test_virtual2.cpp)
svc_target(method_virtual2 GOLDEN method_virtual2.sv)

add_executable(method_virtual3 test_virtual3.cpp)
svc_target(method_virtual3 GOLDEN method_virtual3.sv)

add_executable(method_virtual4 test_virtual4.cpp)
svc_target(method_virtual4 PDF_GRAPH GOLDEN method_virtual4.sv)

add_executable(method_virtual_field test_virtual_field.cpp)
svc_target(method_virtual_field PDF_GRAPH GOLDEN method_virtual_field.sv)

add_executable(method_virtual_cast test_virtual_cast.cpp)
svc_target(method_virtual_cast GOLDEN method_virtual_cast.sv)

add_executable(method_virtual_pure test_virtual_pure.cpp)
svc_target(method_virtual_pure GOLDEN method_virtual_pure.sv)

add_executable(method_if test_if.cpp)
svc_target(method_if GOLDEN method_if.sv)

add_executable(method_switch test_switch.cpp)
svc_target(method_switch GOLDEN method_switch.sv)

add_executable(method_switch_inside test_switch_inside.cpp)
svc_target(method_switch_inside GOLDEN method_switch_inside.sv)

add_executable(method_switch_after test_switch_after.cpp)
svc_target(method_switch_after GOLDEN method_switch_after.sv)

add_executable(method_switch_const test_switch_const.cpp)
svc_target(method_switch_const GOLDEN method_switch_const.sv)

add_executable(method_switch_empty_case test_switch_empty_case.cpp)
svc_target(method_switch_empty_case GOLDEN method_switch_empty_case.sv)

add_executable(method_switch_enum test_switch_enum.cpp)
svc_target(method_switch_enum GOLDEN method_switch_enum.sv)

add_executable(method_switch_empty_fail test_switch_empty_fail.cpp)
svc_target(method_switch_empty_fail WILL_FAIL)

add_executable(method_var test_var.cpp)
svc_target(method_var GOLDEN method_var.sv)

add_executable(method_vname test_vname.cpp)
svc_target(method_vname GOLDEN method_vname.sv)

add_executable(method_var_ref test_var_ref.cpp)
svc_target(method_var_ref GOLDEN method_var_ref.sv)

add_executable(method_var_ref_arr_unkwn test_var_ref_arr_unkwn.cpp)
svc_target(method_var_ref_arr_unkwn GOLDEN method_var_ref_arr_unkwn.sv)

add_executable(method_var_ref_fail test_var_ref_fail.cpp)
svc_target(method_var_ref_fail WILL_FAIL)

add_executable(method_fcall_ref test_fcall_ref.cpp)
svc_target(method_fcall_ref GOLDEN method_fcall_ref.sv)

add_executable(method_fcall_ref_arr_unkwn test_fcall_ref_arr_unkwn.cpp)
svc_target(method_fcall_ref_arr_unkwn GOLDEN method_fcall_ref_arr_unkwn.sv)

add_executable(method_sc_types test_sc_types.cpp)
svc_target(method_sc_types GOLDEN method_sc_types.sv)

add_executable(method_sc_types_fail test_sc_types_fail.cpp)
svc_target(method_sc_types_fail WILL_FAIL)

add_executable(method_sc_bv test_sc_bv.cpp)
svc_target(method_sc_bv GOLDEN method_sc_bv.sv)

add_executable(method_auto_type test_auto_type.cpp)
svc_target(method_auto_type GOLDEN method_auto_type.sv)

add_executable(method_cast test_cast.cpp)
svc_target(method_cast GOLDEN method_cast.sv)


## Binary operations
add_executable(method_binary test_binary.cpp)
svc_target(method_binary GOLDEN method_binary.sv)

add_executable(method_binary_signed test_binary_signed.cpp)
svc_target(method_binary_signed GOLDEN method_binary_signed.sv)

add_executable(method_binary_expr_signed test_binary_expr_signed.cpp)
svc_target(method_binary_expr_signed GOLDEN method_binary_expr_signed.sv)

add_executable(method_binary_signed_fail test_binary_signed_fail.cpp)
svc_target(method_binary_signed_fail)

add_executable(method_binary_sc_types test_binary_sc_types.cpp)
svc_target(method_binary_sc_types GOLDEN method_binary_sc_types.sv)

add_executable(method_compound test_compound.cpp)
svc_target(method_compound GOLDEN method_compound.sv)

add_executable(method_concat test_concat.cpp)
svc_target(method_concat GOLDEN method_concat.sv)

add_executable(method_concat_fail test_concat_fail.cpp)
svc_target(method_concat_fail WILL_FAIL)

add_executable(method_bitwise_compl test_bitwise_compl.cpp)
svc_target(method_bitwise_compl GOLDEN method_bitwise_compl.sv)

add_executable(method_bitwise_not_bool_fail test_bitwise_not_bool_fail.cpp)
svc_target(method_bitwise_not_bool_fail WILL_FAIL)

add_executable(method_mix_signed_unsigned_types test_mix_signed_unsigned_types.cpp)
svc_target(method_mix_signed_unsigned_types GOLDEN method_mix_signed_unsigned_types.sv) 

add_executable(method_shift_type_ext test_shift_type_ext.cpp)
svc_target(method_shift_type_ext GOLDEN method_shift_type_ext.sv)

add_executable(method_shifts_signed test_shifts_signed.cpp)
svc_target(method_shifts_signed GOLDEN method_shifts_signed.sv)

add_executable(method_explicit_type_conv test_explicit_type_conv.cpp)
svc_target(method_explicit_type_conv GOLDEN method_explicit_type_conv.sv)


## Unary operations 
add_executable(method_unary test_unary.cpp)
svc_target(method_unary GOLDEN method_unary.sv)

add_executable(method_unary_sc test_unary_sc.cpp)
svc_target(method_unary_sc GOLDEN method_unary_sc.sv)

# Arrays

add_executable(method_sig_array_access test_sig_array_access.cpp)
svc_target(method_sig_array_access GOLDEN method_sig_array_access.sv)

add_executable(method_enum test_enum.cpp)
svc_target(method_enum GOLDEN method_enum.sv)

add_executable(method_const_array test_const_array.cpp)
svc_target(method_const_array GOLDEN method_const_array.sv)

add_executable(method_pointers test_pointers.cpp)
svc_target(method_pointers GOLDEN method_pointers.sv)

add_executable(method_array test_array.cpp)
svc_target(method_array GOLDEN method_array.sv)

add_executable(method_array_init test_array_init.cpp)
svc_target(method_array_init GOLDEN method_array_init.sv)

add_executable(method_array_param test_array_param.cpp)
svc_target(method_array_param GOLDEN method_array_param.sv)

add_executable(method_array_return test_array_return.cpp)
svc_target(method_array_return GOLDEN method_array_return.sv)

add_executable(method_array_return_fail test_array_return_fail.cpp)
svc_target(method_array_return_fail WILL_FAIL)

add_executable(method_array_unknown test_array_unknown.cpp)
svc_target(method_array_unknown GOLDEN method_array_unknown.sv)

add_executable(method_array_in_if test_array_in_if.cpp)
svc_target(method_array_in_if GOLDEN method_array_in_if.sv)

add_executable(method_var_multiple_use test_var_multiple_use.cpp)
svc_target(method_var_multiple_use  
           GOLDEN method_var_multiple_use.sv)

add_executable(method_var_multiple_use2 test_var_multiple_use2.cpp)
svc_target(method_var_multiple_use2  
           GOLDEN method_var_multiple_use2.sv)

add_executable(method_var_multiple_use3 test_var_multiple_use3.cpp)
svc_target(method_var_multiple_use3  
           GOLDEN method_var_multiple_use3.sv)

add_executable(method_var_usedef_same_proc test_var_usedef_same_proc.cpp)
svc_target(method_var_usedef_same_proc  
           GOLDEN method_var_usedef_same_proc.sv)


## Constant propagation
add_executable(method_const_prop test_const_prop.cpp)
svc_target(method_const_prop GOLDEN method_const_prop.sv)

add_executable(method_const_prop_loop test_const_prop_loop.cpp)
svc_target(method_const_prop_loop GOLDEN method_const_prop_loop.sv)

add_executable(method_const_prop_range test_const_prop_range.cpp)
svc_target(method_const_prop_range GOLDEN method_const_prop_range.sv)

add_executable(method_const_prop_fail test_const_prop_fail.cpp)
svc_target(method_const_prop_fail WILL_FAIL)

add_executable(method_const_prop_fail2 test_const_prop_fail2.cpp)
svc_target(method_const_prop_fail2 WILL_FAIL)


add_executable(method_port_array_bind test_port_array_bind.cpp)
svc_target(method_port_array_bind)

add_executable(method_standard_collection test_standard_collection.cpp)
svc_target(method_standard_collection)

add_executable(method_std_array test_std_array.cpp)
svc_target(method_std_array GOLDEN method_std_array.sv)


## Empty sensitivity
add_executable(method_empty_sensvty test_empty_sensvty.cpp)
svc_target(method_empty_sensvty GOLDEN method_empty_sensvty.sv)

add_executable(method_empty_sensvty1_fail test_empty_sensvty1_fail.cpp)
svc_target(method_empty_sensvty1_fail WILL_FAIL)

add_executable(method_empty_sensvty2_fail test_empty_sensvty2_fail.cpp)
svc_target(method_empty_sensvty2_fail WILL_FAIL)

add_executable(method_empty_sensvty3_fail test_empty_sensvty3_fail.cpp)
svc_target(method_empty_sensvty3_fail WILL_FAIL)

add_executable(method_empty_sensvty4_fail test_empty_sensvty4_fail.cpp)
svc_target(method_empty_sensvty4_fail WILL_FAIL)

add_executable(method_empty_sensvty_dupl_fail test_empty_sensvty_dupl_fail.cpp)
svc_target(method_empty_sensvty_dupl_fail WILL_FAIL)

add_executable(method_non_sensvty_fail test_non_sensvty_fail.cpp)
svc_target(method_non_sensvty_fail WILL_FAIL)

add_executable(method_non_sensvty_arr_fail test_non_sensvty_arr_fail.cpp)
svc_target(method_non_sensvty_arr_fail WILL_FAIL)


add_executable(method_bool test_bool.cpp)
svc_target(method_bool GOLDEN method_bool.sv)

# This test access child module filed which is not supported
#add_executable(method_child_module test_child_module.cpp)
#svc_target(method_child_module GOLDEN method_child_module.sv WILL_FAIL)

add_executable(method_latch test_latch.cpp)
svc_target(method_latch GOLDEN method_latch.sv)

add_executable(method_latch_fail test_latch_fail.cpp)
svc_target(method_latch_fail WILL_FAIL)

add_executable(method_forloop_other_types test_forloop_other_types.cpp)
svc_target(method_forloop_other_types GOLDEN method_forloop_other_types.sv)

add_executable(method_explicit_specialz_templ test_explicit_specialz_templ.cpp)
svc_target(method_explicit_specialz_templ GOLDEN method_explicit_specialz_templ.sv)

## Conditions
add_executable(method_cond_side_effect test_cond_side_effect.cpp)
svc_target(method_cond_side_effect WILL_FAIL)

add_executable(method_cond_oper test_cond_oper.cpp)
svc_target(method_cond_oper GOLDEN method_cond_oper.sv)

add_executable(method_cond_cast test_cond_cast.cpp)
svc_target(method_cond_cast GOLDEN method_cond_cast.sv)

add_executable(method_loop_cond_fail test_loop_cond_fail.cpp)
svc_target(method_loop_cond_fail WILL_FAIL)

add_executable(method_const_compl_cond test_const_compl_cond.cpp)
svc_target(method_const_compl_cond GOLDEN method_const_compl_cond.sv)

add_executable(method_const_cond_narrow test_const_cond_narrow.cpp)
svc_target(method_const_cond_narrow GOLDEN method_const_cond_narrow.sv)

## Decoder And Encoders
add_executable(method_decoder test_decoder.cpp)
svc_target(method_decoder GOLDEN method_decoder.sv)
## Flip Flop And Latches
add_executable(method_ff_latch test_ff_latch.cpp)
svc_target(method_ff_latch GOLDEN method_ff_latch.sv)

add_executable(method_bit_range_access test_bit_range_access.cpp)
svc_target(method_bit_range_access GOLDEN method_bit_range_access.sv)

add_executable(method_bit_range_fail test_bit_range_fail.cpp)
svc_target(method_bit_range_fail WILL_FAIL)

add_executable(method_unsupport_types_fail test_unsupport_types_fail.cpp)
svc_target(method_unsupport_types_fail WILL_FAIL)

## Not supported yet, see #304
#add_executable(method_clock_sensitive test_clock_sensitive.cpp)
#svc_target(method_clock_sensitive GOLDEN method_clock_sensitive.sv)
