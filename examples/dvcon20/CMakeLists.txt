add_executable(dvcon_simple test_simple.cpp)
svc_target(dvcon_simple GOLDEN dvcon_simple.sv)

add_executable(dvcon_fifo test_fifo.cpp)
svc_target(dvcon_fifo GOLDEN dvcon_fifo.sv)

