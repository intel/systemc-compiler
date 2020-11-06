#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

add_executable(test_state_init test_state_init.cpp)
svc_target(test_state_init PDF_GRAPH)

add_executable(test_state_bases test_state_bases.cpp)
svc_target(test_state_bases PDF_GRAPH)

add_executable(test_state_array_int test_state_array_int.cpp)
svc_target(test_state_array_int PDF_GRAPH)

add_executable(test_state_static_const test_state_static_const.cpp)
svc_target(test_state_static_const PDF_GRAPH)

add_executable(test_state_sc_vector test_state_sc_vector.cpp)
svc_target(test_state_sc_vector PDF_GRAPH)

add_executable(test_state_dynalloc test_state_dynalloc.cpp)
svc_target(test_state_dynalloc PDF_GRAPH)

add_executable(test_state_struct_member test_state_struct_member.cpp)
svc_target(test_state_struct_member PDF_GRAPH)

add_executable(test_state_pointers test_state_pointers.cpp)
svc_target(test_state_pointers PDF_GRAPH)

add_executable(test_state_array_el_ptr test_state_array_el_ptr.cpp)
svc_target(test_state_array_el_ptr PDF_GRAPH)

add_executable(test_state_dyn_sig test_state_dyn_sig.cpp)
svc_target(test_state_dyn_sig PDF_GRAPH)

add_executable(test_state_mdim_array test_state_mdim_array.cpp)
svc_target(test_state_mdim_array)

