set(CMAKE_CXX_FLAGS "")

add_executable(test_apb_dma apb_dma_unity.cpp)
target_include_directories(test_apb_dma PRIVATE hls .)
svc_target(test_apb_dma ELAB_ONLY)
