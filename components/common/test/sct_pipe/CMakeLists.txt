#
# Single source testbench for sct_pipe and others
#

if (RTL_MODE_TESTS)
    add_executable(sct_pipe_single-rtl sc_main.cpp)
    target_compile_definitions(sct_pipe_single-rtl PUBLIC SINGLE)
    add_test(NAME sct_pipe_single-rtl COMMAND sct_pipe_single-rtl)
endif()

if (TLM_MODE_TESTS)
    add_executable(sct_pipe_single-tlm sc_main.cpp)
    target_compile_definitions(sct_pipe_single-tlm PUBLIC SINGLE SCT_TLM_MODE)
    add_test(NAME sct_pipe_single-tlm COMMAND sct_pipe_single-tlm)
endif()
