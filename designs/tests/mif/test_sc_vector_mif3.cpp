/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Multiple sc_vector`s of MIF 

struct Producer : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_signal<bool>   req{"req"};
    
    SC_HAS_PROCESS(Producer);
    
    Producer(sc_module_name) 
    {
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<16>> s{"s"};
    
    void threadProc() {
        s = 0; req = 0;
        wait();
        
        while (true) {
            req = 0;
            s = s.read() + 1;
            if (s.read() % 3) req = 1;
            wait();
        }
    }
    
    sc_uint<16> getData() {
        return s.read();
    }
    
};

struct Consumer : sc_module, sc_interface {

    sc_in<bool>    req{"req"};
    sc_out<unsigned>   resp{"resp"};
    
    SC_HAS_PROCESS(Consumer);
    
    Consumer(sc_module_name) {
        SC_METHOD(methProc);
        sensitive << req;
    }
    
    void methProc() {
        resp = 1;
        if (req) resp = 2;
    }
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned M = 2;
    
    Producer p{"p"};
    sc_vector<Producer> pp{"pp", M};
    sc_vector<Consumer> cc{"cc", M};
    
    sc_vector<sc_signal<unsigned>> resp{"resp", M};
    
    SC_CTOR(Top) 
    {
        p.clk(clk); p.rstn(rstn);

        for (int i = 0; i < M; ++i) {
            pp[i].clk(clk); pp[i].rstn(rstn);
            
            cc[i].req(pp[i].req);
            cc[i].resp(resp[i]);
        }
        
        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<16>> s{"s"};
    
    void mainProc() {
        wait();
        
        while (true) {
            unsigned u = s.read();
            sc_uint<16> res = pp[u].getData() + p.getData();
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
