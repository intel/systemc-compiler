/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// static constexpr char not supported more, use std::string instead
struct my_module : public sc_module {
    
    sc_in<bool> in;

    static constexpr char __SC_TOOL_VERILOG_MOD__[] = R"(
module my_module(input in);
   // Some verilog code
endmodule
    )";
    
    SC_CTOR(my_module)
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
