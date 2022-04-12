/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Verilog external module (empty intrinsic) with given module name
template<unsigned N>
struct memory_stub : public sc_module 
{
    sc_in<bool> clk{"clk"};

    // Initialized to empty string
    std::string __SC_TOOL_VERILOG_MOD__ = "";
    
    std::string __SC_TOOL_MODULE_NAME__;
    
    explicit memory_stub(const sc_module_name& name, 
                         const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {
    }

};

// Verilog intrinsic module with given module name
template<unsigned N>
struct memory_stub_intr : public sc_module 
{
    sc_in<bool> clk{"clk"};

    // Initialized to empty string
    std::string __SC_TOOL_VERILOG_MOD__ = 
    R"(
module intrinsic_name(input clk);
endmodule
    )";
    
    const char* __SC_TOOL_MODULE_NAME__;
    
    explicit memory_stub_intr(const sc_module_name& name, 
                              const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {
    }

};

// Generated module (no intrinsic) with given module name
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
    memory_stub<1>          stubInst1;
    memory_stub<1>          stubInst2;
    memory_stub_intr<1>     stubInst3;
    memory_stub_flat<1>     stubInst4;
    memory_stub_flat<1>     stubInst5;

    SC_CTOR(module_with_memory) : 
        stubInst1("stubInst1", "external_name") ,
        stubInst2("stubInst2"),                     // Empty name
        stubInst3("stubInst3", "intrinsic_name"),
        stubInst4("stubInst4", "flat_name"),
        stubInst5("stubInst5", "flat_name1")        // Another name
    {
        stubInst1.clk(clk);
        stubInst2.clk(clk);
        stubInst3.clk(clk);
        stubInst4.clk(clk);
        stubInst5.clk(clk);
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    module_with_memory mod{"mod"};
    mod.clk(clock_gen);
    sc_start();

    return 0;
}

