add_executable(mif_simple test_simple.cpp)
svc_target(mif_simple GOLDEN mif_simple.sv)

add_executable(mif_sc_vector test_sc_vector.cpp)
svc_target(mif_sc_vector GOLDEN mif_sc_vector.sv)

add_executable(mif_array_sc_vector test_array_sc_vector.cpp)
svc_target(mif_array_sc_vector GOLDEN mif_array_sc_vector.sv)

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

## Fix me, error not reported yet, see #115
#add_executable(mif_array_heterogenous test_array_heterogenous.cpp)
#svc_target(mif_array_heterogenous WILL_FAIL)

##Fix me, error not reported yet, see #121
#add_executable(mif_access_violation1 test_access_violation1.cpp)
#svc_target(mif_access_violation1 WILL_FAIL)

## Hangs up, see #114
#add_executable(mif_sc_vector_sc_vector test_sc_vector_sc_vector.cpp)
#svc_target(mif_sc_vector_sc_vector GOLDEN mif_sc_vector_sc_vector.sv)

