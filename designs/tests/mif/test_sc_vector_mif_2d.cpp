/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// 2D sc_vector`s of MIF 

struct Producer : public sc_module, public sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_signal<bool>   req{"req"};
    
    SC_HAS_PROCESS(Producer);
    
    Producer(sc_module_name) 
    {
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    int m;
    unsigned a[2];
    sc_signal<sc_uint<16>>              s{"s"};
    sc_vector<sc_signal<sc_uint<4>>>    sv{"sv", 2};
    sc_signal<sc_uint<5>>               sa[2];
    
    void threadProc() {
        s = 0; req = 0;
        wait();
        
        while (true) {
            req = 0;
            s = s.read() + 1;
            if (s.read() % 3) req = 1;
            m = s.read();
            sv[s.read()] = sa[a[m]].read();
            wait();
        }
    }
    
    sc_uint<16> getData() {
        return s.read();
    }
    
};

struct Consumer : public sc_module, public sc_interface {

    sc_in<bool>         req{"req"};
    sc_out<unsigned>    resp{"resp"};
    
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
    const static unsigned N = 3;
    
    sc_vector<sc_vector<Producer>> pp{"pp", M};
    sc_vector<sc_vector<Consumer>> cc{"cc", M};
    sc_vector<sc_vector<sc_signal<unsigned>>> resp{"resp", M};
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < M; ++i) {
            pp[i].init(N);
            cc[i].init(N);
            resp[i].init(N);
            
            for (int j = 0; j < N; ++j) {
                pp[i][j].clk(clk); pp[i][j].rstn(rstn);
                cc[i][j].req(pp[i][j].req);
                cc[i][j].resp(resp[i][j]);
            }
        }
        
        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<16>> t{"t"};
    
    void mainProc() {
        wait();
        
        while (true) {
            unsigned u = t.read();
            pp[0][0].m = 1;
            pp[u][u+1].m = pp[u][1].m;
            pp[0][u].m = pp[u][u].m;
            
            pp[0][u].a[0] = 1;
            pp[0][u].a[u] = 2;
            pp[u+1][u+1].a[u+1] = pp[u][u].a[u];
            
            pp[0][0].s = pp[0][0].sa[0].read();
            pp[u][u].s = pp[u][u].sa[u+1].read();
            pp[u][u].sa[1] = pp[u][u].sv[u].read();
            pp[u][u].sv[u] = pp[u][u+1].s.read();
            
            //sc_uint<16> res = pp[u][u+1].getData();
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
