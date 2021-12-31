/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Array and pointer array of constants and variables in sc_vector of MIF 
// accessed from MIF and parent processes
template <unsigned N>
struct Producer : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    unsigned*    req[N];
    sc_uint<16> data[N];
    const unsigned C[2] = {11, 22};
    
    SC_HAS_PROCESS(Producer);
    
    Producer(sc_module_name) 
    {
        for(int i = 0; i < N; ++i) {
            req[i] = sc_new<unsigned>(0);
        }
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<16>> s{"s"};
    
    void threadProc() {
        for(int i = 0; i < N; ++i) {
            data[i] = 0;
            *req[i] = 1;
        }
        wait();
        
        while (true) {
            for(int i = 0; i < N; ++i) {
                data[i] = *req[i] ? i : C[i % 2];
                (*req[i])++;
            }
            s = data[s.read()];
            wait();
        }
    }
    
    sc_uint<16> getData(int indx) {
        return (s.read()+indx);
    }
    
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned N = 3;
    const static unsigned M = 2;
    sc_vector<Producer<N>> p{"prod", M};
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < M; ++i) {
            p[i].clk(clk);
            p[i].rstn(rstn);
        }
        
        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<unsigned> s{"s"};
    
    void mainProc() {
        wait();
        
        while (true) {
            unsigned u = s.read();
            sc_uint<16> res = p[u].getData(u+1);

            wait();
            res++;
        }
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);
    Top top("top");
    top.clk(clk);
    
    sc_start();

    return 0;
}
