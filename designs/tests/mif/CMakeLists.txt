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


## Constant analysis for MIF array elements for @generate block
add_executable(mif_array_differ test_mif_array_differ.cpp)
svc_target(mif_array_differ GOLDEN mif_array_differ.sv)

add_executable(mif_array_differ1 test_mif_array_differ.cpp)
target_compile_definitions(mif_array_differ1 PUBLIC OPT1)
svc_target(mif_array_differ1 GOLDEN mif_array_differ1.sv)

add_executable(mif_array_differ2 test_mif_array_differ.cpp)
target_compile_definitions(mif_array_differ2 PUBLIC OPT2)
svc_target(mif_array_differ2 GOLDEN mif_array_differ2.sv)

add_executable(mif_array_differ3 test_mif_array_differ.cpp)
target_compile_definitions(mif_array_differ3 PUBLIC OPT3)
svc_target(mif_array_differ3 GOLDEN mif_array_differ3.sv)

add_executable(mif_array_differ4 test_mif_array_differ.cpp)
target_compile_definitions(mif_array_differ4 PUBLIC OPT1 OPT2 OPT3)
svc_target(mif_array_differ4 GOLDEN mif_array_differ4.sv)

add_executable(mif_array_differ_rec test_mif_array_differ_rec.cpp)
svc_target(mif_array_differ_rec GOLDEN mif_array_differ_rec.sv)

add_executable(mif_array_differ_locparam test_mif_array_differ_locparam.cpp)
svc_target(mif_array_differ_locparam GOLDEN mif_array_differ_locparam.sv)

add_executable(mif_array_differ_mif_ptr test_mif_array_differ_mif_ptr.cpp)
svc_target(mif_array_differ_mif_ptr GOLDEN mif_array_differ_mif_ptr.sv)

add_executable(mif_array_differ_mif_ptr2 test_mif_array_differ_mif_ptr2.cpp)
svc_target(mif_array_differ_mif_ptr2 GOLDEN mif_array_differ_mif_ptr2.sv)


## sc_vector/array in MIF
add_executable(mif_sc_vector test_sc_vector.cpp)
svc_target(mif_sc_vector GOLDEN mif_sc_vector.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_array_sc_vector test_array_sc_vector.cpp)
svc_target(mif_array_sc_vector GOLDEN mif_array_sc_vector.sv NO_REMOVE_EXTRA_CODE)

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
svc_target(mif_name_conflict1 GOLDEN mif_name_conflict1.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_name_conflict2 test_name_conflict2.cpp)
svc_target(mif_name_conflict2 GOLDEN mif_name_conflict2.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_name_conflict3 test_name_conflict3.cpp)
svc_target(mif_name_conflict3 GOLDEN mif_name_conflict3.sv NO_REMOVE_EXTRA_CODE)


add_executable(mif_array1 test_mif_array1.cpp)
svc_target(mif_array1 ELAB_TOP tb_inst.top_inst GOLDEN mif_array1.sv)

add_executable(mif_array2 test_mif_array2.cpp)
svc_target(mif_array2 ELAB_TOP tb_inst.top_inst GOLDEN mif_array2.sv)

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
svc_target(mif_array_with_chan ELAB_TOP tb_inst.top_inst GOLDEN mif_array_with_chan.sv)

add_executable(mif_inherit1 test_mif_inherit1.cpp)
svc_target(mif_inherit1 ELAB_TOP tb_inst.top_inst GOLDEN mif_inherit1.sv)


## MIF with CTHREAD
add_executable(cthread_mif test_cthread_mif.cpp)
svc_target(cthread_mif GOLDEN cthread_mif.sv)

add_executable(cthread_mif_array_fcall test_cthread_mif_array_fcall.cpp)
svc_target(cthread_mif_array_fcall GOLDEN cthread_mif_array_fcall.sv NO_REMOVE_EXTRA_CODE)

add_executable(cthread_mif_array_ptr test_cthread_mif_array_ptr.cpp)
svc_target(cthread_mif_array_ptr GOLDEN cthread_mif_array_ptr.sv NO_REMOVE_EXTRA_CODE)

add_executable(cthread_mif_array_ptr2 test_cthread_mif_array_ptr2.cpp)
svc_target(cthread_mif_array_ptr2  GOLDEN cthread_mif_array_ptr2.sv NO_REMOVE_EXTRA_CODE)

add_executable(cthread_mif_array_ptr_unkwn test_cthread_mif_array_ptr_unkwn.cpp)
svc_target(cthread_mif_array_ptr_unkwn GOLDEN cthread_mif_array_ptr_unkwn.sv NO_REMOVE_EXTRA_CODE)

add_executable(cthread_mif_array_ptr_unkwn2 test_cthread_mif_array_ptr_unkwn2.cpp)
svc_target(cthread_mif_array_ptr_unkwn2 GOLDEN cthread_mif_array_ptr_unkwn2.sv NO_REMOVE_EXTRA_CODE)

add_executable(cthread_mif_array_ptr_unkwn3 test_cthread_mif_array_ptr_unkwn3.cpp)
svc_target(cthread_mif_array_ptr_unkwn3 GOLDEN cthread_mif_array_ptr_unkwn3.sv NO_REMOVE_EXTRA_CODE)

## MIF with METHOD
add_executable(mif_array_with_ports test_mif_array_with_ports.cpp)
svc_target(mif_array_with_ports GOLDEN mif_array_with_ports.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_array_with_ptr test_mif_array_with_ptr.cpp)
svc_target(mif_array_with_ptr GOLDEN mif_array_with_ptr.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_array_with_ptr2 test_mif_array_with_ptr2.cpp)
svc_target(mif_array_with_ptr2 GOLDEN mif_array_with_ptr2.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_array_with_ptr3 test_mif_array_with_ptr3.cpp)
svc_target(mif_array_with_ptr3 GOLDEN mif_array_with_ptr3.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_array_with_ptr_array test_mif_array_with_ptr_array.cpp)
svc_target(mif_array_with_ptr_array GOLDEN mif_array_with_ptr_array.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_var_multiple_use test_mif_var_multiple_use.cpp)
svc_target(mif_var_multiple_use WILL_FAIL)

## Fix me, error not reported yet, see #115
#add_executable(mif_array_heterogenous test_array_heterogenous.cpp)
#svc_target(mif_array_heterogenous WILL_FAIL)

##Fix me, error not reported yet, see #121
#add_executable(mif_access_violation1 test_access_violation1.cpp)
#svc_target(mif_access_violation1 WILL_FAIL)

## sc_vector of MIF
add_executable(mif_sc_vector_mif1 test_sc_vector_mif1.cpp)
svc_target(mif_sc_vector_mif1 GOLDEN mif_sc_vector_mif1.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_mif2 test_sc_vector_mif2.cpp)
svc_target(mif_sc_vector_mif2 GOLDEN mif_sc_vector_mif2.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_mif3 test_sc_vector_mif3.cpp)
svc_target(mif_sc_vector_mif3 GOLDEN mif_sc_vector_mif3.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_mif4 test_sc_vector_mif4.cpp)
svc_target(mif_sc_vector_mif4 GOLDEN mif_sc_vector_mif4.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_mif5 test_sc_vector_mif5.cpp)
svc_target(mif_sc_vector_mif5 GOLDEN mif_sc_vector_mif5.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_mif6 test_sc_vector_mif6.cpp)
svc_target(mif_sc_vector_mif6 GOLDEN mif_sc_vector_mif6.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_ptr test_sc_vector_ptr.cpp)
svc_target(mif_sc_vector_ptr GOLDEN mif_sc_vector_ptr.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_mif_2d test_sc_vector_mif_2d.cpp)
svc_target(mif_sc_vector_mif_2d GOLDEN mif_sc_vector_mif_2d.sv NO_REMOVE_EXTRA_CODE)

add_executable(mif_sc_vector_mif_simple test_sc_vector_mif_simple.cpp)
svc_target(mif_sc_vector_mif_simple GOLDEN mif_sc_vector_mif_simple.sv)

add_executable(mif_sc_vector_mif_simple2D test_sc_vector_mif_simple.cpp)
target_compile_definitions(mif_sc_vector_mif_simple2D PUBLIC VEC_2D)
svc_target(mif_sc_vector_mif_simple2D GOLDEN mif_sc_vector_mif_simple2D.sv)

add_executable(mif_glob_const test_glob_const.cpp)
svc_target(mif_glob_const GOLDEN mif_glob_const.sv)

add_executable(dttc2021 test_dttc2021.cpp)
svc_target(dttc2021 GOLDEN dttc2021.sv)

## MIF/MIF array inside of MIF array
add_executable(mif_array_with_mif test_mif_array_with_mif.cpp)
svc_target(mif_array_with_mif ELAB_TOP top GOLDEN mif_array_with_mif.sv)

add_executable(mif_array_with_mif_array test_mif_array_with_mif_array.cpp)
svc_target(mif_array_with_mif_array ELAB_TOP top GOLDEN mif_array_with_mif_array.sv)

## UseDef tests
add_executable(mif_use_def1 test_use_def1.cpp)
svc_target(mif_use_def1 GOLDEN mif_use_def1.sv)

add_executable(mif_use_def2 test_use_def2.cpp)
svc_target(mif_use_def2 GOLDEN mif_use_def2.sv)

add_executable(mif_use_def3 test_use_def3.cpp)
svc_target(mif_use_def3 GOLDEN mif_use_def3.sv)
