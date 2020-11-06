/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Channel used/defined in the same method process error detecting test
struct Child : public sc_module
{
    sc_in_clk               clk;
    sc_in<bool>             rst;

    sc_in<int>              in;     
    sc_out<int>             out;
    sc_signal<int>          s;
    sc_signal<int>          t;

    SC_CTOR(Child)
    {
        SC_METHOD(methA);
        sensitive << in << s;

        SC_METHOD(methB);
        sensitive << s << out << in;
    }

    void methA() {
        s = in.read();
        t = s;
    }

    void methB() 
    {
        if (s) {
            int j = out.read();
        }
        out = in.read();
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
