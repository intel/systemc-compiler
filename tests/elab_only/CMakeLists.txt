#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(simple_test simple_test.cpp)
svc_target(simple_test ELAB_ONLY)

add_executable(test_array test_array.cpp)
svc_target(test_array ELAB_ONLY )

add_executable(test_ctor_param test_ctor_param.cpp)
svc_target(test_ctor_param ELAB_ONLY)

add_executable(test_ctor_name test_ctor_name.cpp)
svc_target(test_ctor_name GOLDEN test_ctor_name.sv)

add_executable(test_large_array test_large_array.cpp)
svc_target(test_large_array ELAB_ONLY)

add_executable(test_many_procs test_many_procs.cpp)
svc_target(test_many_procs ELAB_ONLY )

add_executable(test_members test_members.cpp)
svc_target(test_members ELAB_ONLY)

add_executable(test_procs test_procs.cpp)
svc_target(test_procs ELAB_ONLY )

add_executable(test_sc_vector test_sc_vector.cpp)
svc_target(test_sc_vector ELAB_ONLY)

add_executable(test_std_array test_std_array.cpp)
svc_target(test_std_array ELAB_ONLY)

add_executable(test_template_params test_template_params.cpp)
svc_target(test_template_params ELAB_ONLY)

add_executable(test_master_slave test_master_slave.cpp)
svc_target(test_master_slave ELAB_ONLY )

add_executable(test_virtual_ports test_virtual_ports.cpp)
svc_target(test_virtual_ports ELAB_ONLY )

add_executable(test_virtual_ports2 test_virtual_ports2.cpp)
svc_target(test_virtual_ports2 ELAB_ONLY )

add_executable(test_virtual_ports3 test_virtual_ports3.cpp)
svc_target(test_virtual_ports3 ELAB_ONLY )

add_executable(test_internal_port_not_bound test_internal_port_not_bound.cpp)
svc_target(test_internal_port_not_bound GOLDEN test_internal_port_not_bound.sv)

add_executable(test_port_not_bound test_port_not_bound.cpp)
svc_target(test_port_not_bound ELAB_ONLY  WILL_FAIL)

add_executable(test_port_not_bound2 test_port_not_bound2.cpp)
svc_target(test_port_not_bound2 ELAB_ONLY  WILL_FAIL)

add_executable(test_port_not_bound3 test_port_not_bound3.cpp)
svc_target(test_port_not_bound3 ELAB_ONLY  WILL_FAIL)

add_executable(test_virtual_ports_cross test_virtual_ports_cross.cpp)
svc_target(test_virtual_ports_cross ELAB_ONLY )

add_executable(test_static_member test_static_member.cpp)
svc_target(test_static_member ELAB_ONLY )

add_executable(test_raw_new test_raw_new.cpp)
svc_target(test_raw_new ELAB_ONLY )

add_executable(test_template_new test_template_new.cpp)
svc_target(test_template_new ELAB_ONLY )

add_executable(test_base_class test_base_class.cpp)
svc_target(test_base_class ELAB_ONLY )

add_executable(test_reference test_reference.cpp)
svc_target(test_reference ELAB_ONLY )

add_executable(test_virtual_base test_virtual_base.cpp)
svc_target(test_virtual_base ELAB_ONLY )

add_executable(test_bind_same test_bind_same.cpp)
svc_target(test_bind_same ELAB_ONLY )

add_executable(test_bind_up test_bind_up.cpp)
svc_target(test_bind_up ELAB_ONLY )

add_executable(test_bind_down test_bind_down.cpp)
svc_target(test_bind_down ELAB_ONLY )

add_executable(test_bind_cross test_bind_cross.cpp)
svc_target(test_bind_cross ELAB_ONLY )

add_executable(test_gdb_bug test_gdb_bug.cpp)
svc_target(test_gdb_bug ELAB_ONLY )

add_executable(test_base_scmethod test_base_scmethod.cpp)
svc_target(test_base_scmethod ELAB_ONLY )

add_executable(test_base_scmethod2 test_base_scmethod2.cpp)
svc_target(test_base_scmethod2 )

add_executable(test_modular_if0 test_modular_if0.cpp)
svc_target(test_modular_if0 ELAB_ONLY )

add_executable(test_modif_vector test_modif_vector.cpp)
svc_target(test_modif_vector ELAB_ONLY )

add_executable(test_sig_array test_sig_array.cpp)
svc_target(test_sig_array ELAB_ONLY )

add_executable(test_swg_demo test_swg_demo.cpp)
svc_target(test_swg_demo ELAB_ONLY )

add_executable(test_array_of_ptr test_array_of_ptr.cpp)
svc_target(test_array_of_ptr ELAB_ONLY )

add_executable(test_adder test_adder.cpp)
svc_target(test_adder  ELAB_ONLY )

add_executable(test_local_data test_local_data.cpp)
svc_target(test_local_data ELAB_ONLY )

add_executable(test_unique_ptr test_unique_ptr.cpp)
svc_target(test_unique_ptr ELAB_ONLY )

add_executable(dvcon_demo dvcon_demo.cpp)
svc_target(dvcon_demo ELAB_ONLY )

add_executable(test_no_top test_no_top.cpp)
svc_target(test_no_top ELAB_ONLY WILL_FAIL)

add_executable(test_multiple_top test_multiple_top.cpp)
svc_target(test_multiple_top ELAB_ONLY WILL_FAIL)

add_executable(test_scelab2_types test_scelab2_types.cpp)
svc_target(test_scelab2_types)

add_executable(test_phase_callback test_phase_callback.cpp)
svc_target(test_phase_callback)

add_executable(test_sc_find_object test_sc_find_object.cpp)
svc_target(test_sc_find_object)

add_executable(test_bind_array_by_element test_bind_array_by_element.cpp)
svc_target(test_bind_array_by_element ELAB_TOP w.t)

add_executable(test_sparse_arrays test_sparse_arrays.cpp)
svc_target(test_sparse_arrays GOLDEN test_sparse_arrays.sv)

add_executable(test_bind_array_to_array test_bind_array_to_array.cpp)
svc_target(test_bind_array_to_array)

add_executable(test_sensitive_keep_arrays test_sensitive_keep_arrays.cpp)
svc_target(test_sensitive_keep_arrays GOLDEN test_sensitive_keep_arrays.sv)


