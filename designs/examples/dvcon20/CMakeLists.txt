add_executable(dvcon_simple test_simple.cpp)
svc_target(dvcon_simple ELAB_TOP tb.dut)

add_executable(dvcon_fifo test_fifo.cpp)
svc_target(dvcon_fifo ELAB_TOP tb.dut GOLDEN dvcon_fifo.sv)

