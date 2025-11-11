#
# Single source library testbench 
#

add_executable(sct_portmap sc_main.cpp)
#target_compile_definitions(sct_portmap PUBLIC -DMETHOD)
svc_target(sct_portmap PORT_MAP_GENERATE ELAB_TOP test_top.dut_inst)

