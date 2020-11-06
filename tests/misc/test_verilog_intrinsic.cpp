/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Intrinsic modified in module constructor
struct my_module : public sc_module {
    
    sc_in<bool> in;

    std::string __SC_TOOL_VERILOG_MOD__;
    
    SC_CTOR(my_module) :
        __SC_TOOL_VERILOG_MOD__(
    R"(
module my_module(input in);
   // Some verilog code
endmodule
    )")
    {}
    
};


SC_MODULE(testbench) 
{
    sc_signal<bool> sig;
    
    my_module a;

    SC_CTOR(testbench) : a("a") {
        a.in(sig);
    }
};

int sc_main(int argc, char **argv) {

    testbench tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
