add_executable(mif_simple test_simple.cpp)
svc_target(mif_simple GOLDEN mif_simple.sv)

add_executable(mif_const1 test_mif_const1.cpp)
svc_target(mif_const1 GOLDEN mif_const1.sv)

add_executable(mif_const2 test_mif_const2.cpp)
svc_target(mif_const2 GOLDEN mif_const2.sv)

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

add_executable(mif_name_conflict1 test_name_conflict1.cpp)
svc_target(mif_name_conflict1 GOLDEN mif_name_conflict1.sv)

add_executable(mif_name_conflict2 test_name_conflict2.cpp)
svc_target(mif_name_conflict2 GOLDEN mif_name_conflict2.sv)

add_executable(mif_name_conflict3 test_name_conflict3.cpp)
svc_target(mif_name_conflict3 GOLDEN mif_name_conflict3.sv)

