/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// sc_vector of MIF which has MIF inside

struct A : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_signal<unsigned>  resp{"resp"};
    
    SC_HAS_PROCESS(A);
    
    A(sc_module_name) 
    {
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void threadProc() {
        resp = 0;
        wait();
        
        while (true) {
            resp = resp.read() + 1;
            wait();
        }
    }
    
    unsigned getData() {
        return resp.read();
    }
    
};

struct B : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;
    sc_in<bool>  req{"req"};
    
    A  mif_a{"mif_a"};
    
    SC_HAS_PROCESS(B);
    
    B(sc_module_name) 
    {
        mif_a.clk(clk);
        mif_a.rstn(rstn);
        
        SC_METHOD(methProc);
        sensitive << mif_a.resp << req;
    }
    
    sc_signal<unsigned> res{"res"};
    void methProc() {
        res = 0;
        if (req) res = mif_a.getData() % 3;
    }
    
    unsigned getRes() {
        return res.read();
    }
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned M = 2;
    
    sc_vector<B> pp{"pp", M};
    
    sc_vector<sc_signal<bool>> req{"req", M};
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < M; ++i) {
            pp[i].clk(clk); pp[i].rstn(rstn);
            pp[i].req(req[i]);
        }
        
        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<16>> s{"s"};
    
    void mainProc() {
        wait();
        
        while (true) {
            unsigned res = pp[s.read()].getRes();
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
