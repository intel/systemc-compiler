/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Array of ports in top module interface
struct Top : sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    
    static const unsigned N = 3;
    
    sc_vector<sc_in<bool>>             in{"in", N};
    sc_vector<sc_out<sc_uint<4>>>      out{"out", N};
    sc_vector<sc_signal<sc_uint<4>>>   sig{"sig", N};

    SC_CTOR(Top) {
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_METHOD(methProc);
        sensitive << sig[1] << sig[2];
    }
    
    void methProc() {
        sig[0] = 1;
        sig[2] = 2;
        out[0] = sig[1].read() + sig[2].read();
    }
    
    void threadProc() 
    {
        for (int i = 1; i < N; ++i) {
            out[i] = 0;
        }
        wait();
        
        while(true) {
            
            for (int i = 1; i < N; ++i) {
                out[i] = in[i] ? sig[i].read() : (sc_uint<4>)0;
            }
            
            wait();
        }
    }
    
};

int sc_main(int argc, char** argv)
{
    Top top{"top"};
    
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> rst;
    sc_vector<sc_signal<bool>> a{"a", 3};
    sc_vector<sc_signal<sc_uint<4>>> b{"b", 3};
    
    top.clk(clk);
    top.rst(rst);
    for (int i = 0; i < 3; ++i) {
        top.in[i](a[i]);
        top.out[i](b[i]);
    }
    
    sc_start();
    return 0;
}

