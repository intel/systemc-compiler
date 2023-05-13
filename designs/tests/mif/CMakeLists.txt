add_executable(mif_simple1 test_mif_simple1.cpp)
svc_target(mif_simple1 GOLDEN mif_simple1.sv)

add_executable(mif_simple2 test_mif_simple2.cpp)
svc_target(mif_simple2 GOLDEN mif_simple2.sv)

add_executable(mif_simple3 test_mif_simple3.cpp)
svc_target(mif_simple3 ELAB_TOP tb_inst.top_inst GOLDEN mif_simple3.sv)

add_executable(mif_simple4 test_mif_simple4.cpp)
svc_target(mif_simple4 ELAB_TOP tb_inst.top_inst GOLDEN mif_simple4.sv)

add_executable(mif_simple5 test_mif_simple5.cpp)
svc_target(mif_simple5 ELAB_TOP tb_inst.top_inst GOLDEN mif_simple5.sv)

add_executable(mif_const1 test_mif_const1.cpp)
svc_target(mif_const1 GOLDEN mif_const1.sv)

add_executable(mif_const2 test_mif_const2.cpp)
svc_target(mif_const2 GOLDEN mif_const2.sv)

add_executable(mif_array_differ test_mif_array_differ.cpp)
svc_target(mif_array_differ GOLDEN mif_array_differ.sv)

## sc_vector/array in MIF
add_executable(mif_sc_vector test_sc_vector.cpp)
svc_target(mif_sc_vector GOLDEN mif_sc_vector.sv)

add_executable(mif_array_sc_vector test_array_sc_vector.cpp)
svc_target(mif_array_sc_vector GOLDEN mif_array_sc_vector.sv)

add_executable(mif_with_chan_array test_mif_with_chan_array.cpp)
svc_target(mif_with_chan_array GOLDEN mif_with_chan_array.sv)

add_executable(mif_var_in_reset test_var_in_reset.cpp)
svc_target(mif_var_in_reset GOLDEN mif_var_in_reset.sv)

add_executable(mif_record_meth test_record_meth.cpp)
svc_target(mif_record_meth GOLDEN mif_record_meth.sv)

add_executable(mif_record_cthread test_record_cthread.cpp)
svc_target(mif_record_cthread GOLDEN mif_record_cthread.sv)

add_executable(mif_array_record_meth test_array_record_meth.cpp)
svc_target(mif_array_record_meth GOLDEN mif_array_record_meth.sv)

add_executable(mif_array_record_cthread test_array_record_cthread.cpp)
svc_target(mif_array_record_cthread GOLDEN mif_array_record_cthread.sv)


add_executable(mif_name_conflict1 test_name_conflict1.cpp)
svc_target(mif_name_conflict1 GOLDEN mif_name_conflict1.sv)

add_executable(mif_name_conflict2 test_name_conflict2.cpp)
svc_target(mif_name_conflict2 GOLDEN mif_name_conflict2.sv)

add_executable(mif_name_conflict3 test_name_conflict3.cpp)
svc_target(mif_name_conflict3 GOLDEN mif_name_conflict3.sv)


add_executable(mif_array1 test_mif_array1.cpp)
svc_target(mif_array1 GOLDEN mif_array1.sv)

add_executable(mif_array2 test_mif_array2.cpp)
svc_target(mif_array2 GOLDEN mif_array2.sv)

add_executable(mif_array3 test_mif_array3.cpp)
svc_target(mif_array3 GOLDEN mif_array3.sv)

add_executable(mif_array4 test_mif_array4.cpp)
svc_target(mif_array4 GOLDEN mif_array4.sv)

add_executable(mif_array_sens test_mif_array_sens.cpp)
svc_target(mif_array_sens GOLDEN mif_array_sens.sv)

add_executable(mif_array_ptr test_mif_array_ptr.cpp)
svc_target(mif_array_ptr GOLDEN mif_array_ptr.sv)

add_executable(mif_array_ptr2 test_mif_array_ptr2.cpp)
svc_target(mif_array_ptr2 GOLDEN mif_array_ptr2.sv)

add_executable(mif_array_unkwn test_mif_array_unkwn.cpp)
svc_target(mif_array_unkwn GOLDEN mif_array_unkwn.sv)

add_executable(mif_array_const_unkwn test_mif_array_const_unkwn.cpp)
svc_target(mif_array_const_unkwn GOLDEN mif_array_const_unkwn.sv)

add_executable(mif_array_with_chan test_mif_array_with_chan.cpp)
svc_target(mif_array_with_chan GOLDEN mif_array_with_chan.sv)

add_executable(mif_inherit1 test_mif_inherit1.cpp)
svc_target(mif_inherit1 GOLDEN mif_inherit1.sv)


## MIF with CTHREAD
add_executable(cthread_mif test_cthread_mif.cpp)
svc_target(cthread_mif GOLDEN cthread_mif.sv)

add_executable(cthread_mif_array_fcall test_cthread_mif_array_fcall.cpp)
svc_target(cthread_mif_array_fcall GOLDEN cthread_mif_array_fcall.sv)

add_executable(cthread_mif_array_ptr test_cthread_mif_array_ptr.cpp)
svc_target(cthread_mif_array_ptr GOLDEN cthread_mif_array_ptr.sv)

add_executable(cthread_mif_array_ptr2 test_cthread_mif_array_ptr2.cpp)
svc_target(cthread_mif_array_ptr2  GOLDEN cthread_mif_array_ptr2.sv)

add_executable(cthread_mif_array_ptr_unkwn test_cthread_mif_array_ptr_unkwn.cpp)
svc_target(cthread_mif_array_ptr_unkwn GOLDEN cthread_mif_array_ptr_unkwn.sv)

add_executable(cthread_mif_array_ptr_unkwn2 test_cthread_mif_array_ptr_unkwn2.cpp)
svc_target(cthread_mif_array_ptr_unkwn2 GOLDEN cthread_mif_array_ptr_unkwn2.sv)

add_executable(cthread_mif_array_ptr_unkwn3 test_cthread_mif_array_ptr_unkwn3.cpp)
svc_target(cthread_mif_array_ptr_unkwn3 GOLDEN cthread_mif_array_ptr_unkwn3.sv)

## MIF with METHOD
add_executable(mif_array_with_ports test_mif_array_with_ports.cpp)
svc_target(mif_array_with_ports GOLDEN mif_array_with_ports.sv)

add_executable(mif_array_with_ptr test_mif_array_with_ptr.cpp)
svc_target(mif_array_with_ptr GOLDEN mif_array_with_ptr.sv)

add_executable(mif_array_with_ptr2 test_mif_array_with_ptr2.cpp)
svc_target(mif_array_with_ptr2 GOLDEN mif_array_with_ptr2.sv)

add_executable(mif_array_with_ptr3 test_mif_array_with_ptr3.cpp)
svc_target(mif_array_with_ptr3 GOLDEN mif_array_with_ptr3.sv)

add_executable(mif_array_with_ptr_array test_mif_array_with_ptr_array.cpp)
svc_target(mif_array_with_ptr_array GOLDEN mif_array_with_ptr_array.sv)

add_executable(mif_var_multiple_use test_mif_var_multiple_use.cpp)
svc_target(mif_var_multiple_use GOLDEN mif_var_multiple_use.sv)

## Fix me, error not reported yet, see #115
#add_executable(mif_array_heterogenous test_array_heterogenous.cpp)
#svc_target(mif_array_heterogenous WILL_FAIL)

##Fix me, error not reported yet, see #121
#add_executable(mif_access_violation1 test_access_violation1.cpp)
#svc_target(mif_access_violation1 WILL_FAIL)

## sc_vector of MIF
add_executable(mif_sc_vector_mif1 test_sc_vector_mif1.cpp)
svc_target(mif_sc_vector_mif1 GOLDEN mif_sc_vector_mif1.sv)

add_executable(mif_sc_vector_mif2 test_sc_vector_mif2.cpp)
svc_target(mif_sc_vector_mif2 GOLDEN mif_sc_vector_mif2.sv)

add_executable(mif_sc_vector_mif3 test_sc_vector_mif3.cpp)
svc_target(mif_sc_vector_mif3 GOLDEN mif_sc_vector_mif3.sv)

add_executable(mif_sc_vector_mif4 test_sc_vector_mif4.cpp)
svc_target(mif_sc_vector_mif4 GOLDEN mif_sc_vector_mif4.sv)

add_executable(mif_sc_vector_mif5 test_sc_vector_mif5.cpp)
svc_target(mif_sc_vector_mif5 GOLDEN mif_sc_vector_mif5.sv)

add_executable(mif_sc_vector_mif6 test_sc_vector_mif6.cpp)
svc_target(mif_sc_vector_mif6 GOLDEN mif_sc_vector_mif6.sv)

add_executable(mif_sc_vector_ptr test_sc_vector_ptr.cpp)
svc_target(mif_sc_vector_ptr GOLDEN mif_sc_vector_ptr.sv)

add_executable(mif_sc_vector_mif_2d test_sc_vector_mif_2d.cpp)
svc_target(mif_sc_vector_mif_2d GOLDEN mif_sc_vector_mif_2d.sv)

add_executable(mif_glob_const test_glob_const.cpp)
svc_target(mif_glob_const GOLDEN mif_glob_const.sv)

add_executable(dttc2021 test_dttc2021.cpp)
svc_target(dttc2021 GOLDEN dttc2021.sv)
