/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include "sct_assert.h"

template <unsigned N>
struct Producer : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_vector<sc_in<bool>>  req{"req", N};
    sc_vector<sc_signal<sc_uint<16>>> data{"data", N};

    SC_HAS_PROCESS(Producer);
    
    Producer(sc_module_name) 
    {
        //SC_CTHREAD(threadProc, clk.pos());
        //async_reset_signal_is(rstn, false);
        
        //SC_METHOD(methProc);
        //for (int i = 0; i < N; ++i) sensitive << req[i];
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

// Check design statistic report
SC_MODULE(test_stat) 
{
    sc_in<bool>     clk;
    sc_signal<bool> rstn;
    
    sc_signal<bool> sig{"sig"};
    sc_signal<bool> sigArray[2];

    int x;
    int &xref = x;
    
    const static unsigned N = 3;
    sc_vector<sc_signal<bool>> req{"req", N};
    
    const static unsigned M = 1;
    sc_vector<Producer<N>> p{"pp", M};

    SC_CTOR(test_stat) 
    {
        for (int i = 0; i < M; ++i) {
            //p[i] = new Producer<N>("pp");
            p[i].clk(clk);
            p[i].rstn(rstn);
            p[i].req.bind(req);
        }
        
        //SC_METHOD(test_method);
        //sensitive << sig;
        
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(rstn, 0);
    }

    void test_method() {
        int j;
        x = sig.read();
        xref = 2;
        sigArray[0] = 1;
        
        if (sig.read()) {
            int i = 0;
        }
        
//        do {
//            x--;
//        } while (x > 0);
    }
    
    void test_thread() {
        int state = 0;
        wait();
        while (true) {
            //state += 1;
            sig = state;
            //sct_assert (state);
            while(!sig) wait();
            wait();
        }
    }

};

int sc_main(int argc, char **argv) 
{
    sc_clock clk("clk", 1, SC_NS);
    test_stat tinst{"tinst"};
    tinst.clk(clk);
    sc_start();
    return 0;
}
