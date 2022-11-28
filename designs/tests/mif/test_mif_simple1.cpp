/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Modular interface without processes
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<bool>     in;
    
    static const unsigned A = 42;
    sc_uint<16>     b;

    SC_CTOR(mod_if) 
    {}
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    mod_if          minst{"minst"};
    
    sc_signal<bool> s;

    SC_CTOR(Top) {
        minst.clk(clk);
        minst.in(s);
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock  clk("clk", sc_time(1, SC_NS));
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
