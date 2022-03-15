/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// sc_vector of channels in sc_vector of MIF accessed from MIF and parent processes
template <unsigned N>
struct Producer : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_vector<sc_in<bool>>  req{"req", N};
    sc_vector<sc_signal<sc_uint<16>>> data{"data", N};

    SC_HAS_PROCESS(Producer);
    
    Producer(sc_module_name) 
    {
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
//        SC_METHOD(methProc);
//        for (int i = 0; i < N; ++i) sensitive << req[i];
    }
    
    void threadProc() {
        for(int i = 0; i < N; ++i) {
            data[i] = 0;
        }
        wait();
        
        while (true) {
            for(int i = 0; i < N; ++i) {
                data[i] = req[i] ? i : 0;
            }
            wait();
        }
    }
    
    void methProc() {
        for(int i = 0; i < N; ++i) {
            data[i] = req[i] ? i : 0;
        }
    }
    
    sc_uint<16> getData(int par) {
        return data[par].read();
    }
    
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned N = 3;
    sc_vector<sc_signal<bool>> req{"req", N};

    const static unsigned M = 2;
    sc_vector<Producer<N>> p{"pp", M};
    //Producer<N>* p[M];
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < M; ++i) {
            //p[i] = new Producer<N>("pp");
            p[i].clk(clk);
            p[i].rstn(rstn);
            p[i].req.bind(req);
        }
        
        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void mainProc() {
        for(int i = 0; i < N; ++i) {
            req[i] = 0;
        }
        wait();
        
        while (true) {
            for(int i = 0; i < M; ++i) {
                for(int j = 0; j < N; ++j) {
                    sc_uint<16> x = p[i].getData(j); 
                }
            }

            wait();
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
