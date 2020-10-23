#set(COMMON_HOME $ENV{ICSC_HOME}/svc/components/common)
set(COMMON_TEST $ENV{ICSC_HOME}/svc/components/common/test)
#include_directories(
#    ${COMMON_HOME} 
#    ${COMMON_HOME}/sctcommon
#)

add_executable(misc_verilog_intrinsic test_verilog_intrinsic.cpp)
svc_target(misc_verilog_intrinsic GOLDEN misc_verilog_intrinsic.sv)

add_executable(misc_verilog_intrinsic2 test_verilog_intrinsic2.cpp)
svc_target(misc_verilog_intrinsic2 GOLDEN misc_verilog_intrinsic2.sv)

add_executable(misc_verilog_intrinsic3 test_verilog_intrinsic3.cpp)
svc_target(misc_verilog_intrinsic3 GOLDEN misc_verilog_intrinsic3.sv)

add_executable(misc_verilog_intrinsic_fail test_verilog_intrinsic_fail.cpp)
svc_target(misc_verilog_intrinsic_fail WILL_FAIL)

add_executable(misc_promote_ports test_promote_ports.cpp)
svc_target(misc_promote_ports ELAB_TOP tb_inst.top0 GOLDEN misc_promote_ports.sv)

# Run the same test with another @ELAB_TOP
add_executable(misc_promote_ports_2 test_promote_ports.cpp)
svc_target(misc_promote_ports_2 ELAB_TOP tb_inst.top0.inner0 
           GOLDEN misc_promote_ports_2.sv)

add_executable(misc_reference_met test_reference_met.cpp)
svc_target(misc_reference_met)

add_executable(misc_single_state_thread test_single_state_thread.cpp)
svc_target(misc_single_state_thread GOLDEN misc_single_state_thread.sv)

## Arrays
add_executable(misc_module_port_array test_module_port_array.cpp)
svc_target(misc_module_port_array GOLDEN misc_module_port_array.sv)

add_executable(misc_dynamic_ports test_dynamic_ports.cpp)
svc_target(misc_dynamic_ports GOLDEN misc_dynamic_ports.sv)

add_executable(misc_dynamic_array test_dynamic_array.cpp)
svc_target(misc_dynamic_array GOLDEN misc_dynamic_array.sv)

# These test with not bound ports leads to fatal error -- that is OK
#add_executable(misc_double_alloc test_double_alloc.cpp)
#svc_target(misc_double_alloc GOLDEN misc_double_alloc.sv)

# Will fail as access to child module fields is prohibited
add_executable(misc_array_module_fail test_array_module_fail.cpp)
svc_target(misc_array_module_fail WILL_FAIL)

add_executable(misc_array_module test_array_module.cpp)
svc_target(misc_array_module GOLDEN misc_array_module.sv)

add_executable(misc_array_in_top test_array_in_top.cpp)
svc_target(misc_array_in_top GOLDEN misc_array_in_top.sv)

add_executable(misc_module_base_array test_module_base_array.cpp)
svc_target(misc_module_base_array GOLDEN misc_module_base_array.sv)

add_executable(misc_array_no_indx_fail test_array_no_indx_fail.cpp)
svc_target(misc_array_no_indx_fail WILL_FAIL)

add_executable(misc_const_array test_const_array.cpp)
svc_target(misc_const_array GOLDEN misc_const_array.sv)

## sct_comb_signal and memory
add_executable(misc_vendor_memory test_vendor_memory.cpp)
svc_target(misc_vendor_memory GOLDEN misc_vendor_memory.sv)

add_executable(misc_comb_signal test_comb_signal.cpp)
svc_target(misc_comb_signal ELAB_TOP tb.mod
           GOLDEN misc_comb_signal.sv)

add_executable(misc_comb_signal_clear test_comb_signal_clear.cpp)
svc_target(misc_comb_signal_clear ELAB_TOP tb.mod
           GOLDEN misc_comb_signal_clear.sv)

add_executable(misc_comb_signal_fail test_comb_signal_fail.cpp)
svc_target(misc_comb_signal_fail ELAB_TOP tb.mod)

add_executable(misc_string test_string.cpp) 
svc_target(misc_string GOLDEN misc_string.sv)

add_executable(misc_vendor_memory_sem test_vendor_memory_sem.cpp)
svc_target(misc_vendor_memory_sem GOLDEN misc_vendor_memory_sem.sv)

add_executable(misc_vendor_memory_if test_vendor_memory_if.cpp)
svc_target(misc_vendor_memory_if GOLDEN misc_vendor_memory_if.sv)

## ...
add_executable(misc_module_typedef test_module_typedef.cpp)
svc_target(misc_module_typedef GOLDEN misc_module_typedef.sv)

add_executable(misc_cross_hierarchy_bind1 test_cross_hierarchy_bind1.cpp)
svc_target(misc_cross_hierarchy_bind1 
           GOLDEN misc_cross_hierarchy_bind1.sv)

add_executable(misc_cross_hierarchy_bind2 test_cross_hierarchy_bind2.cpp)
svc_target(misc_cross_hierarchy_bind2 
           GOLDEN misc_cross_hierarchy_bind2.sv)

add_executable(misc_cross_hierarchy_bind3 test_cross_hierarchy_bind3.cpp)
svc_target(misc_cross_hierarchy_bind3 
           GOLDEN misc_cross_hierarchy_bind3.sv)

## Constants
add_executable(misc_const_init test_const_init.cpp)
svc_target(misc_const_init GOLDEN misc_const_init.sv)

add_executable(misc_const_radix test_const_radix.cpp)
svc_target(misc_const_radix GOLDEN misc_const_radix.sv)

add_executable(misc_const_remove test_const_remove.cpp)
svc_target(misc_const_remove GOLDEN misc_const_remove.sv)

add_executable(misc_const_pointer test_const_pointer.cpp)
svc_target(misc_const_pointer GOLDEN misc_const_pointer.sv)

## Pointers
add_executable(misc_multiple_pointers_var test_multiple_pointers_var.cpp)
svc_target(misc_multiple_pointers_var GOLDEN misc_multiple_pointers_var.sv)

add_executable(misc_multiple_pointers_mod test_multiple_pointers_mod.cpp)
svc_target(misc_multiple_pointers_mod GOLDEN misc_multiple_pointers_mod.sv)

add_executable(misc_array_to_pointer test_array_to_pointer.cpp)
svc_target(misc_array_to_pointer GOLDEN misc_array_to_pointer.sv)

add_executable(misc_pointer_warning test_pointer_warning.cpp)
svc_target(misc_pointer_warning GOLDEN misc_pointer_warning.sv)

add_executable(misc_pointer_null_fail test_pointer_null_fail.cpp)
svc_target(misc_pointer_null_fail ELAB_TOP b_mod.a_mod WILL_FAIL)

add_executable(misc_pointer_null2_fail test_pointer_null2_fail.cpp)
svc_target(misc_pointer_null2_fail ELAB_TOP b_mod.a_mod WILL_FAIL)

add_executable(misc_pointer_null3_fail test_pointer_null3_fail.cpp)
svc_target(misc_pointer_null3_fail 
           ELAB_TOP b_mod.a_mod WILL_FAIL)

add_executable(misc_pointer_null_arr_fail test_pointer_null_arr_fail.cpp)
svc_target(misc_pointer_null_arr_fail 
           ELAB_TOP b_mod.a_mod WILL_FAIL)

add_executable(misc_pointer_dangling_fail test_pointer_dangling_fail.cpp)
svc_target(misc_pointer_dangling_fail ELAB_TOP b_mod.a_mod WILL_FAIL)

add_executable(misc_pointer_dangling2_fail test_pointer_dangling2_fail.cpp)
svc_target(misc_pointer_dangling2_fail ELAB_TOP b_mod.a_mod WILL_FAIL)

#add_executable(misc_pointer_dangling3_fail test_pointer_dangling3_fail.cpp)
#svc_target(misc_pointer_dangling3_fail ELAB_TOP b_mod.a_mod WILL_FAIL)

## SC ports
add_executable(misc_sc_port test_sc_port.cpp)
svc_target(misc_sc_port GOLDEN misc_sc_port.sv)

## Fix me, see #115
#add_executable(misc_sc_port2 test_sc_port2.cpp)
#svc_target(misc_sc_port2 GOLDEN misc_sc_port2.sv)

## Fix me, see #116
#add_executable(misc_sc_port_array1 test_sc_port_array1.cpp)
#svc_target(misc_sc_port_array1 GOLDEN misc_sc_port_array1.sv)

## Module port array not supported yet
#add_executable(misc_sc_port_array2 test_sc_port_array2.cpp)
#svc_target(misc_sc_port_array2 GOLDEN misc_sc_port_array2.sv)

## Fix me, see #118
#add_executable(misc_sc_port3 test_sc_port3.cpp)
#svc_target(misc_sc_port3 GOLDEN misc_sc_port3.sv)

## Fix me, see #118
#add_executable(misc_sc_port4 test_sc_port4.cpp)
#svc_target(misc_sc_port4 GOLDEN misc_sc_port4.sv)

add_executable(misc_sc_port_dyn test_sc_port_dyn.cpp)
svc_target(misc_sc_port_dyn GOLDEN misc_sc_port_dyn.sv)

add_executable(misc_sc_port_ptr test_sc_port_ptr.cpp)
svc_target(misc_sc_port_ptr GOLDEN misc_sc_port_ptr.sv)

add_executable(misc_sc_port_ptr_dyn test_sc_port_ptr_dyn.cpp)
svc_target(misc_sc_port_ptr_dyn GOLDEN misc_sc_port_ptr_dyn.sv)

add_executable(test_dac2019_apb test_dac2019_apb.cpp)
svc_target(test_dac2019_apb)

add_executable(test_demo_nov27 test_demo_nov27.cpp)
svc_target(test_demo_nov27 ELAB_TOP top.demo0)

add_executable(misc_sva_assert test_sva_assert.cpp)
svc_target(misc_sva_assert GOLDEN misc_sva_assert.sv ELAB_TOP test_top.a_mod)

add_executable(misc_sva_generate test_sva_generate.cpp)
svc_target(misc_sva_generate ELAB_TOP test_top.a_mod)

add_executable(misc_sva_generate_mod test_sva_generate_mod.cpp)
svc_target(misc_sva_generate_mod GOLDEN misc_sva_generate_mod.sv 
           ELAB_TOP test_top.a_mod)

add_executable(misc_sva_generate_base_mod test_sva_generate_base_mod.cpp)
svc_target(misc_sva_generate_base_mod GOLDEN misc_sva_generate_base_mod.sv 
           ELAB_TOP test_top.a_mod)

add_executable(misc_sva_generate_mod_fail test_sva_generate_mod_fail.cpp)
svc_target(misc_sva_generate_mod_fail ELAB_TOP test_top.a_mod WILL_FAIL)

add_executable(misc_sva_generate_proc test_sva_generate_proc.cpp)
svc_target(misc_sva_generate_proc GOLDEN misc_sva_generate_proc.sv 
           ELAB_TOP test_top.a_mod)

add_executable(misc_sva_generate_proc_loop test_sva_generate_proc_loop.cpp)
svc_target(misc_sva_generate_proc_loop GOLDEN misc_sva_generate_proc_loop.sv 
           ELAB_TOP test_top.a_mod)

add_executable(misc_sva_generate_proc_fail test_sva_generate_proc_fail.cpp)
svc_target(misc_sva_generate_proc_fail ELAB_TOP test_top.a_mod WILL_FAIL)

add_executable(misc_sva_generate_func_fail1 test_sva_generate_func_fail1.cpp)
svc_target(misc_sva_generate_func_fail1 WILL_FAIL)

add_executable(misc_sva_generate_func_fail2 test_sva_generate_func_fail2.cpp)
svc_target(misc_sva_generate_func_fail2 WILL_FAIL)

add_executable(misc_sva_generate_proc_nogen test_sva_generate_proc_nogen.cpp)
svc_target(misc_sva_generate_proc_nogen NO_SVA_GENERATE 
           GOLDEN misc_sva_generate_proc_nogen.sv 
           ELAB_TOP test_top.a_mod)

add_executable(misc_sva_generate_proc_off test_sva_generate_proc_off.cpp)
target_compile_definitions(misc_sva_generate_proc_off PUBLIC -DSCT_ASSERT_OFF)
svc_target(misc_sva_generate_proc_off GOLDEN misc_sva_generate_proc_off.sv 
           ELAB_TOP test_top.a_mod)

# Example for UG
add_executable(misc_promote_ports_simple test_promote_ports_simple.cpp)
svc_target(misc_promote_ports_simple ELAB_TOP tb_inst.top_mod 
           GOLDEN misc_promote_ports_simple.sv)

add_executable(misc_module_binds_simple test_module_binds_simple.cpp)
svc_target(misc_module_binds_simple ELAB_TOP tb_inst.top_mod 
           GOLDEN misc_module_binds_simple.sv)

add_executable(misc_sc_port_ptr2 test_sc_port_ptr2.cpp)
svc_target(misc_sc_port_ptr2 GOLDEN misc_sc_port_ptr2.sv)

add_executable(misc_process_simple test_process_simple.cpp)
svc_target(misc_process_simple ELAB_TOP tb_inst.top_mod 
           GOLDEN misc_process_simple.sv)

add_executable(misc_const_enum_namespace test_const_enum_namespace.cpp)
svc_target(misc_const_enum_namespace ELAB_TOP tb_inst.top_mod 
           GOLDEN misc_const_enum_namespace.sv)

add_executable(misc_const_global test_const_global.cpp)
svc_target(misc_const_global ELAB_TOP tb_inst.top_mod 
           GOLDEN misc_const_global.sv)

# Inner records not supported yet
#add_executable(misc_reg_array_target test_reg_array_target.cpp)
#svc_target(misc_reg_array_target ELAB_TOP top 
#           GOLDEN misc_reg_array_target.sv)

add_executable(misc_read_notdef_var test_read_notdef_var.cpp)
svc_target(misc_read_notdef_var GOLDEN misc_read_notdef_var.sv)

add_executable(misc_read_notdef_chan test_read_notdef_chan.cpp)
svc_target(misc_read_notdef_chan GOLDEN misc_read_notdef_chan.sv)

add_executable(misc_verilog_keyword test_verilog_keyword.cpp)
svc_target(misc_verilog_keyword GOLDEN misc_verilog_keyword.sv)

# Function calls
add_executable(misc_fcall_params test_fcall_params.cpp)
svc_target(misc_fcall_params GOLDEN misc_fcall_params.sv)

add_executable(misc_fcall_array test_fcall_array.cpp)
svc_target(misc_fcall_array GOLDEN misc_fcall_array.sv)

add_executable(misc_fcall_empty test_fcall_empty.cpp)
svc_target(misc_fcall_empty GOLDEN misc_fcall_empty.sv) 

add_executable(misc_fcall_array_unknown test_fcall_array_unknown.cpp)
svc_target(misc_fcall_array_unknown GOLDEN misc_fcall_array_unknown.sv)

add_executable(misc_fcall_multi_state test_fcall_multi_state.cpp)
svc_target(misc_fcall_multi_state GOLDEN misc_fcall_multi_state.sv) 


add_executable(misc_large_array test_large_array.cpp)
svc_target(misc_large_array)

add_executable(misc_sc_vector1 test_sc_vector1.cpp)
svc_target(misc_sc_vector1 GOLDEN misc_sc_vector1.sv)

add_executable(misc_sc_vector2 test_sc_vector2.cpp)
svc_target(misc_sc_vector2 GOLDEN misc_sc_vector2.sv)

add_executable(misc_sig_access_reset test_sig_access_reset.cpp)
svc_target(misc_sig_access_reset GOLDEN misc_sig_access_reset.sv)

add_executable(misc_array_ptr_noninit test_array_ptr_noninit.cpp)
svc_target(misc_array_ptr_noninit GOLDEN misc_array_ptr_noninit.sv)

add_executable(misc_zero_width_fail test_zero_width_fail.cpp)
svc_target(misc_zero_width_fail WILL_FAIL)

add_executable(misc_neg_range_fail test_neg_range_fail.cpp)
svc_target(misc_neg_range_fail WILL_FAIL)

add_executable(misc_init_local_var test_init_local_var.cpp)
svc_target(misc_init_local_var GOLDEN misc_init_local_var.sv INIT_LOCAL_VARS)

