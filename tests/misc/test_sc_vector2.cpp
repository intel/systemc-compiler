/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Vector of channels (sc_vector) including vector binds test
template <unsigned N>
struct Producer : sc_module {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_vector<sc_in<bool>>  req{"req", N};
    sc_vector<sc_out<sc_uint<16>>>  data{"data", N};
    sc_vector<sc_vector<sc_in<int>>>  req2D{"req2D", N};

    SC_HAS_PROCESS(Producer);
    
    Producer(sc_module_name) 
    {
        for (int i = 0; i < N; ++i) {
            req2D[i].init(2);
        }
        
        SC_METHOD(methProc);
        for (int i = 0; i < N; ++i) {
            sensitive << req[i] << req2D[i][0];
        }
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<bool> greq;
    
    void methProc() 
    {
        bool a = 0;
        for(int i = 0; i < N; ++i) {
            a = a || req[i] || req2D[i][0];
        }
        greq = a;
    }
    
    void threadProc() {
        sc_uint<4> n = 1;
        data[0] = 0;
        wait();
        
        while (true) {
            if (greq) {
                for(int i = 1; i < N; ++i) {
                    data[i] = data[i-1];
                }
            }
            data[0] = n++;
            wait();
        }
    }
    
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned N = 3;
    sc_vector<sc_signal<bool>> req{"req", N};
    sc_vector<sc_signal<sc_uint<16>>> data{"data", N};
    sc_vector<sc_vector<sc_signal<int>>>  req2D{"req2D", N};

    Producer<N> p{"p"};
    
    SC_CTOR(Top) {
        
        for (int i = 0; i < N; ++i) {
            req2D[i].init(2);
        }
        
        p.clk(clk);
        p.rstn(rstn);
        p.req.bind(req);
        p.data.bind(data);
        p.req2D.bind(req2D);

        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void mainProc() {
        for(int i = 1; i < N; ++i) {
            req[i] = 0;
            req2D[i][0] = i;
        }
        wait();
        
        while (true) {
            req[0] = 1;
            wait();
            req[0] = 0;

            while (!data[1].read()) wait();
            req[1] = 1;
            wait();
            req[1] = 0;
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
