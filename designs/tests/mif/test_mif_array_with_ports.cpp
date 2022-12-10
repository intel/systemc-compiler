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

// Array of modular interface pointers with ports
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk               clk;
    sc_in<bool>             rst;
    
    sc_in<int>              min;
    sc_out<int>             mout;

    sc_in<int>              in;
    sc_signal<int>          sig;
    sc_out<int>             out;
    
    const unsigned A = 1;
    static const unsigned B = 2;
    static constexpr unsigned C = 3;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(meth);
        sensitive << min;

        SC_CTHREAD(thrd, clk.pos());
        async_reset_signal_is(rst, true);
    }

    unsigned m = 0;
    void meth() {
        int i = m + A + B;
        mout = min.read() + i;
    }
    
    unsigned n = 0;
    void thrd() {
        out = 0;
        wait();
        
        while (true) {
            int i = n + A + B;
            int a = in.read() + i;
            wait();
            sig = a;
        }
    }

    int readIn() {
        return in.read();
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  x;
    sc_signal<int>  y;
    sc_signal<int>  t;
    sc_signal<int>  z;
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);
        }
        minst[0]->min.bind(x);
        minst[1]->min.bind(y);
        minst[0]->mout.bind(x);
        minst[1]->mout.bind(y);
        
        minst[0]->in.bind(t);
        minst[1]->in.bind(z);
        minst[0]->out.bind(t);
        minst[1]->out.bind(z);
        
        SC_METHOD(top_meth);
        sensitive << t << y;
    }
    
    void top_meth() {
        int i = t.read() + y.read();
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
