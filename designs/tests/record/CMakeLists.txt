#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(record_reg_cthread0 test_reg_cthread0.cpp)
svc_target(record_reg_cthread0 GOLDEN record_reg_cthread0.sv)

add_executable(record_reg_cthread1 test_reg_cthread1.cpp)
svc_target(record_reg_cthread1 GOLDEN record_reg_cthread1.sv)

add_executable(record_reg_arr_cthread test_reg_arr_cthread.cpp)
svc_target(record_reg_arr_cthread GOLDEN record_reg_arr_cthread.sv)

add_executable(record_reg_func_cthread1 test_reg_func_cthread1.cpp)
svc_target(record_reg_func_cthread1 GOLDEN record_reg_func_cthread1.sv)

add_executable(record_reg_func_cthread2 test_reg_func_cthread2.cpp)
svc_target(record_reg_func_cthread2 GOLDEN record_reg_func_cthread2.sv)

add_executable(record_return_cthread test_return_cthread.cpp)
svc_target(record_return_cthread GOLDEN record_return_cthread.sv)

add_executable(record_return_cthread1 test_return_cthread1.cpp)
svc_target(record_return_cthread1 GOLDEN record_return_cthread1.sv)

add_executable(record_return_cthread2 test_return_cthread2.cpp)
svc_target(record_return_cthread2 GOLDEN record_return_cthread2.sv)

add_executable(record_return_cthread3 test_return_cthread3.cpp)
svc_target(record_return_cthread3 GOLDEN record_return_cthread3.sv)

add_executable(record_return_static_func_cthread test_return_static_func_cthread.cpp)
svc_target(record_return_static_func_cthread GOLDEN record_return_static_func_cthread.sv)

add_executable(record_fcall_cthread test_fcall_cthread.cpp)
svc_target(record_fcall_cthread GOLDEN record_fcall_cthread.sv)

add_executable(record_fcall_glob_cthread test_fcall_glob_cthread.cpp)
svc_target(record_fcall_glob_cthread GOLDEN record_fcall_glob_cthread.sv)

add_executable(record_func_param_cthread test_func_param_cthread.cpp)
svc_target(record_func_param_cthread GOLDEN record_func_param_cthread.sv)

add_executable(record_member_func_cthread test_member_func_cthread.cpp)
svc_target(record_member_func_cthread GOLDEN record_member_func_cthread.sv)

add_executable(record_func_in_ctor test_func_in_ctor.cpp)
svc_target(record_func_in_ctor GOLDEN record_func_in_ctor.sv)

# Inner record not supported
#add_executable(record_member_record_func test_member_record_func.cpp)
#svc_target(record_member_record_func GOLDEN record_member_record_func.sv)

#add_executable(record_member_record1 test_member_record1.cpp)
#svc_target(record_member_record1 GOLDEN record_member_record1.sv)

add_executable(record_member_array_cthread test_member_array_cthread.cpp)
svc_target(record_member_array_cthread GOLDEN record_member_array_cthread.sv)

add_executable(record_member_array_method1 test_member_array_method1.cpp)
svc_target(record_member_array_method1 GOLDEN record_member_array_method1.sv)

add_executable(record_member_array_method2 test_member_array_method2.cpp)
svc_target(record_member_array_method2 GOLDEN record_member_array_method2.sv)

add_executable(record_member_array_method3 test_member_array_method3.cpp)
svc_target(record_member_array_method3 GOLDEN record_member_array_method3.sv)

add_executable(record_member_array_unkwn test_member_array_unkwn.cpp)
svc_target(record_member_array_unkwn GOLDEN record_member_array_unkwn.sv)

add_executable(record_member_array_copy test_member_array_copy.cpp)
svc_target(record_member_array_copy GOLDEN record_member_array_copy.sv)

# Fix me, #80
add_executable(record_array_cthread test_array_cthread.cpp)
svc_target(record_array_cthread GOLDEN record_array_cthread.sv)

add_executable(record_array_unknown_cthread test_array_unknown_cthread.cpp)
svc_target(record_array_unknown_cthread GOLDEN record_array_unknown_cthread.sv)

add_executable(record_member_func_method test_member_func_method.cpp)
svc_target(record_member_func_method GOLDEN record_member_func_method.sv)

add_executable(record_array_fcall_method test_array_fcall_method.cpp)
svc_target(record_array_fcall_method GOLDEN record_array_fcall_method.sv)

add_executable(record_array_if_method test_array_if_method.cpp)
svc_target(record_array_if_method GOLDEN record_array_if_method.sv)

add_executable(record_array_loops_method test_array_loops_method.cpp)
svc_target(record_array_loops_method GOLDEN record_array_loops_method.sv)

add_executable(record_array_method test_array_method.cpp)
svc_target(record_array_method GOLDEN record_array_method.sv)

add_executable(record_array_method2 test_array_method2.cpp)
svc_target(record_array_method2 GOLDEN record_array_method2.sv)

add_executable(record_array_unknown_method test_array_unknown_method.cpp)
svc_target(record_array_unknown_method GOLDEN record_array_unknown_method.sv)

add_executable(record_array_var_indices_method test_array_var_indices_method.cpp)
svc_target(record_array_var_indices_method GOLDEN record_array_var_indices_method.sv)

add_executable(record_array_decl_in_scope test_array_decl_in_scope.cpp)
svc_target(record_array_decl_in_scope GOLDEN record_array_decl_in_scope.sv)

add_executable(record_assign_concat_method test_assign_concat_method.cpp)
svc_target(record_assign_concat_method GOLDEN record_assign_concat_method.sv)

add_executable(record_assign_concat_cthread test_assign_concat_cthread.cpp)
svc_target(record_assign_concat_cthread GOLDEN record_assign_concat_cthread.sv)

add_executable(record_complex_method test_complex_method.cpp)
svc_target(record_complex_method GOLDEN record_complex_method.sv)

add_executable(record_func_param_method test_func_param_method.cpp)
svc_target(record_func_param_method GOLDEN record_func_param_method.sv)

add_executable(record_if_method test_if_method.cpp)
svc_target(record_if_method GOLDEN record_if_method.sv)

add_executable(record_ptr_arr_method test_ptr_arr_method.cpp)
svc_target(record_ptr_arr_method GOLDEN record_ptr_arr_method.sv)

add_executable(record_ptr_arr_cthread test_ptr_arr_cthread.cpp)
svc_target(record_ptr_arr_cthread GOLDEN record_ptr_arr_cthread.sv)

add_executable(record_return_assert_method test_return_assert_method.cpp)
svc_target(record_return_assert_method GOLDEN record_return_assert_method.sv)

add_executable(record_return_method test_return_method.cpp)
svc_target(record_return_method GOLDEN record_return_method.sv)

add_executable(record_simple_method test_simple_method.cpp)
svc_target(record_simple_method GOLDEN record_simple_method.sv)

add_executable(record_simple_method3 test_simple_method3.cpp)
svc_target(record_simple_method3 GOLDEN record_simple_method3.sv)

add_executable(record_read_defined test_read_defined.cpp)
svc_target(record_read_defined GOLDEN record_read_defined.sv)

add_executable(record_base_record0 test_base_record0.cpp)
svc_target(record_base_record0 GOLDEN record_base_record0.sv)

add_executable(record_base_record1 test_base_record1.cpp)
svc_target(record_base_record1 GOLDEN record_base_record1.sv)

add_executable(record_base_record2 test_base_record2.cpp)
svc_target(record_base_record2 GOLDEN record_base_record2.sv)

add_executable(record_base_multi_record test_base_multi_record.cpp)
svc_target(record_base_multi_record GOLDEN record_base_multi_record.sv)

add_executable(record_wait_call test_wait_call.cpp)
svc_target(record_wait_call GOLDEN record_wait_call.sv)

## Record as channel type
add_executable(record_chan_type_meth test_chan_type_meth.cpp)
svc_target(record_chan_type_meth GOLDEN record_chan_type_meth.sv)

add_executable(record_chan_type_thrd test_chan_type_thrd.cpp)
svc_target(record_chan_type_thrd GOLDEN record_chan_type_thrd.sv)

add_executable(record_chan_type_misc test_chan_type_misc.cpp)
svc_target(record_chan_type_misc GOLDEN record_chan_type_misc.sv)

add_executable(record_use_def test_use_def.cpp)
svc_target(record_use_def GOLDEN record_use_def.sv)

