#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Example counter
project(counter)

# All synthesizable source files must be listed here (not in libraries)
add_executable(counter example.cpp)

# Test source directory
target_include_directories(counter PUBLIC $ENV{ICSC_HOME}/examples/counter)

# Add compilation options
# target_compile_definitions(example PUBLIC -DMYOPTION)
# target_compile_options(example PUBLIC -Wall)

# Add optional library, no add SystemC library (it added by svc_target)
#target_link_libraries(example sometestbenchlibrary)

# svc_target will create @example_sctool executable that runs code generation 
# and @example that runs general SystemC simulation
# ELAB_TOP parameter accepts hierarchical name of DUT  
# (that is SystemC name, returned by sc_object::name() method)
svc_target(counter ELAB_TOP tb.dut_inst)