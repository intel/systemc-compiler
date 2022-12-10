/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Vector of channels (sc_vector) in MIF
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
    }
    
    void threadProc() {
        sc_uint<4> n = 1; data[0] = 0;
        wait();
        
        while (true) {
            for(int i = 1; i < N; ++i) {
                data[i] = req[i] ? data[i-1].read() : (sc_uint<16>)0;
            }
            data[0] = n++;
            wait();
        }
    }
    
    sc_uint<16> getData(int i) {
        return data[i].read();
    }
    
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned N = 3;
    sc_vector<sc_signal<bool>> req{"req", N};

    Producer<N> p{"p"};
    
    SC_CTOR(Top) {
        
        p.clk(clk);
        p.rstn(rstn);
        p.req.bind(req);

        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void mainProc() {
        for(int i = 0; i < N; ++i) {
            req[i] = 0;
        }
        wait();
        
        while (true) {
            for(int i = 0; i < N; ++i) {
                sc_uint<16> x = p.getData(i); 
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
