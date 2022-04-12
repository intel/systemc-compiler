/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Memory modules with same name, test failed
template<unsigned N>
struct memory_stub_flat : public sc_module 
{
    sc_in<bool> clk{"clk"};

    std::string __SC_TOOL_MODULE_NAME__;
    
    explicit memory_stub_flat(const sc_module_name& name, 
                              const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {
    }

};

// Generated module (no intrinsic) with given module name
template<unsigned N>
struct memory_stub_flat_ : public sc_module 
{
    sc_in<bool> clk{"clk"};

    std::string __SC_TOOL_MODULE_NAME__;
    
    explicit memory_stub_flat_(const sc_module_name& name, 
                               const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {
    }

};

struct module_with_memory : public sc_module 
{
    sc_in<bool>             clk{"clk"};
    memory_stub_flat<1>     stubInst1;
    memory_stub_flat_<1>    stubInst2;

    SC_CTOR(module_with_memory) : 
        stubInst1("stubInst1", "flat_name"),
        stubInst2("stubInst2", "flat_name")         // Duplicate name
    {
        stubInst1.clk(clk);
        stubInst2.clk(clk);
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    module_with_memory mod{"mod"};
    mod.clk(clock_gen);
    sc_start();

    return 0;
}

