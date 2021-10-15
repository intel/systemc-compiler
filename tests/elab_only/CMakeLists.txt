#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

#add_subdirectory(test_apb_dma)
#add_subdirectory(multi_source)

add_executable(elab_simple_test simple_test.cpp)
svc_target(elab_simple_test ELAB_ONLY)

add_executable(elab_array test_array.cpp)
svc_target(elab_array ELAB_ONLY )

add_executable(elab_ctor_param test_ctor_param.cpp)
svc_target(elab_ctor_param ELAB_ONLY)

add_executable(elab_ctor_name test_ctor_name.cpp)
svc_target(elab_ctor_name GOLDEN elab_ctor_name.sv)

add_executable(elab_large_array test_large_array.cpp)
svc_target(elab_large_array ELAB_ONLY)

add_executable(elab_many_procs test_many_procs.cpp)
svc_target(elab_many_procs ELAB_ONLY )

add_executable(elab_members test_members.cpp)
svc_target(elab_members ELAB_ONLY)

add_executable(elab_procs test_procs.cpp)
svc_target(elab_procs ELAB_ONLY )

add_executable(elab_sc_vector test_sc_vector.cpp)
svc_target(elab_sc_vector ELAB_ONLY)

add_executable(elab_std_array test_std_array.cpp)
svc_target(elab_std_array ELAB_ONLY)

add_executable(elab_template_params test_template_params.cpp)
svc_target(elab_template_params ELAB_ONLY)

add_executable(elab_master_slave test_master_slave.cpp)
svc_target(elab_master_slave ELAB_ONLY )

add_executable(elab_virtual_ports test_virtual_ports.cpp)
svc_target(elab_virtual_ports ELAB_ONLY )

add_executable(elab_virtual_ports2 test_virtual_ports2.cpp)
svc_target(elab_virtual_ports2 ELAB_ONLY )

add_executable(elab_virtual_ports3 test_virtual_ports3.cpp)
svc_target(elab_virtual_ports3 ELAB_ONLY )

add_executable(elab_internal_port_not_bound test_internal_port_not_bound.cpp)
svc_target(elab_internal_port_not_bound GOLDEN elab_internal_port_not_bound.sv)

add_executable(elab_port_not_bound test_port_not_bound.cpp)
svc_target(elab_port_not_bound ELAB_ONLY  WILL_FAIL)

add_executable(elab_port_not_bound2 test_port_not_bound2.cpp)
svc_target(elab_port_not_bound2 ELAB_ONLY  WILL_FAIL)

add_executable(elab_port_not_bound3 test_port_not_bound3.cpp)
svc_target(elab_port_not_bound3 ELAB_ONLY  WILL_FAIL)

add_executable(elab_virtual_ports_cross test_virtual_ports_cross.cpp)
svc_target(elab_virtual_ports_cross ELAB_ONLY )

add_executable(elab_static_member test_static_member.cpp)
svc_target(elab_static_member ELAB_ONLY )

add_executable(elab_raw_new test_raw_new.cpp)
svc_target(elab_raw_new ELAB_ONLY )

add_executable(elab_template_new test_template_new.cpp)
svc_target(elab_template_new ELAB_ONLY )

add_executable(elab_base_class test_base_class.cpp)
svc_target(elab_base_class ELAB_ONLY )

add_executable(elab_reference test_reference.cpp)
svc_target(elab_reference ELAB_ONLY )

add_executable(elab_virtual_base test_virtual_base.cpp)
svc_target(elab_virtual_base ELAB_ONLY )

add_executable(elab_bind_same test_bind_same.cpp)
svc_target(elab_bind_same ELAB_ONLY )

add_executable(elab_bind_up test_bind_up.cpp)
svc_target(elab_bind_up ELAB_ONLY )

add_executable(elab_bind_down test_bind_down.cpp)
svc_target(elab_bind_down ELAB_ONLY )

add_executable(elab_bind_cross test_bind_cross.cpp)
svc_target(elab_bind_cross ELAB_ONLY )

add_executable(elab_gdb_bug test_gdb_bug.cpp)
svc_target(elab_gdb_bug ELAB_ONLY )

add_executable(elab_base_scmethod test_base_scmethod.cpp)
svc_target(elab_base_scmethod ELAB_ONLY )

add_executable(elab_base_scmethod2 test_base_scmethod2.cpp)
svc_target(elab_base_scmethod2 )

add_executable(elab_modular_if0 test_modular_if0.cpp)
svc_target(elab_modular_if0 ELAB_ONLY )

add_executable(elab_modif_vector test_modif_vector.cpp)
svc_target(elab_modif_vector ELAB_ONLY )

add_executable(elab_sig_array test_sig_array.cpp)
svc_target(elab_sig_array ELAB_ONLY )

add_executable(elab_swg_demo test_swg_demo.cpp)
svc_target(elab_swg_demo ELAB_ONLY )

add_executable(elab_array_of_ptr test_array_of_ptr.cpp)
svc_target(elab_array_of_ptr ELAB_ONLY )

add_executable(elab_adder test_adder.cpp)
svc_target(elab_adder  ELAB_ONLY )

add_executable(elab_local_data test_local_data.cpp)
svc_target(elab_local_data ELAB_ONLY )

add_executable(elab_unique_ptr test_unique_ptr.cpp)
svc_target(elab_unique_ptr ELAB_ONLY )

#add_executable(elab_dvcon_demo dvcon_demo.cpp)
#svc_target(elab_dvcon_demo ELAB_ONLY )

add_executable(elab_no_top test_no_top.cpp)
svc_target(elab_no_top ELAB_ONLY WILL_FAIL)

add_executable(elab_multiple_top test_multiple_top.cpp)
svc_target(elab_multiple_top ELAB_ONLY WILL_FAIL)

add_executable(elab_scelab2_types test_scelab2_types.cpp)
svc_target(elab_scelab2_types)

add_executable(elab_phase_callback test_phase_callback.cpp)
svc_target(elab_phase_callback)

add_executable(elab_sc_find_object test_sc_find_object.cpp)
svc_target(elab_sc_find_object)

add_executable(elab_bind_array_by_element test_bind_array_by_element.cpp)
svc_target(elab_bind_array_by_element ELAB_TOP w.t GOLDEN elab_bind_array_by_element.sv)

add_executable(elab_sparse_arrays test_sparse_arrays.cpp)
svc_target(elab_sparse_arrays GOLDEN elab_sparse_arrays.sv)

add_executable(elab_bind_array_to_array test_bind_array_to_array.cpp)
svc_target(elab_bind_array_to_array GOLDEN elab_bind_array_to_array.sv)

add_executable(elab_sensitive_keep_arrays test_sensitive_keep_arrays.cpp)
svc_target(elab_sensitive_keep_arrays GOLDEN elab_sensitive_keep_arrays.sv)


