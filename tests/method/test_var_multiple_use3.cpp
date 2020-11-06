/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 06/14/19.
//

#include <systemc.h>

// Variable used/defined in multiple process error detecting test
// No warning detected by this test (negative test)
struct Child : public sc_module
{
    sc_in_clk               clk;
    sc_in<bool>             rst;

    sc_in<int>              in;     
    sc_out<int>             out;
    sc_signal<int>          s;
    sc_uint<3>              v;
    const int               c;

    SC_CTOR(Child) : c(42)
    {
        SC_METHOD(methA);
        sensitive << in;

        SC_CTHREAD(thrA, clk.pos());
        async_reset_signal_is(rst, true);
    }

    void methA() {
        v = in.read();
        s = 2 - v;
        if (in) {
            s = v + 1;
        }
        s = in.read();
    }

    void thrA() {
        out = 0;
        wait();
        
        while (true) {
            out = in.read();
            wait();
            out = c;
        }
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    Child           child;
    sc_signal<int>  t;      

    SC_CTOR(Top) : child("child") 
    {
        child.clk(clk);
        child.rst(rst);
        child.in(t);
        child.out(t);
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
