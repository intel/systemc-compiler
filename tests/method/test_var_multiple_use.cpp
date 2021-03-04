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
struct Child : public sc_module
{
    sc_in_clk               clk;
    sc_in<bool>             rst;

    sc_in<int>              in;     
    sc_out<int>             out;
    sc_signal<int>          s;
    sc_uint<3>              v;
    sc_uint<3>              vv;
    const int               c;

    SC_CTOR(Child) : v(21), c(v % 11)
    {
        SC_METHOD(methA);
        sensitive << in;

        SC_METHOD(methB);
        sensitive << in;

        SC_METHOD(methC);
        sensitive << in;
    }

    void methA() {
        if (in)
            v = c;
        s = 2 - v;
    }

    void methB() {
        out = in.read() + vv;
    }

    void methC() {
        if (in)
            out = 0;
        else 
            out = 1;
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
