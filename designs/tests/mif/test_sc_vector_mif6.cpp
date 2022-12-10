/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// sc_vector of MIF which has sc_vector of MIF inside, member written in parent thread processes 

struct A : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_in<bool>  req;
    sc_signal<unsigned>  resp{"resp"};
    sc_signal<unsigned>  a{"a"};
    sc_signal<unsigned>  b{"b"};
    unsigned m;
    unsigned n;
    
    SC_HAS_PROCESS(A);
    
    A(sc_module_name) 
    {}
    
};

struct B : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;
    sc_in<bool>  req{"req"};
    
    const static unsigned M = 3;
    A mif_aa{"mif_aa"};
    sc_vector<A> mif_a{"mif_a", M};
    
    SC_HAS_PROCESS(B);
    
    B(sc_module_name) 
    {
        mif_aa.clk(clk); 
        mif_aa.rstn(rstn);
        mif_aa.req(req);
        for (int i = 0; i < M; ++i) {
            mif_a[i].clk(clk); 
            mif_a[i].rstn(rstn);
            mif_a[i].req(req);
        }
        
        SC_METHOD(methB);
        sensitive << c;
        
        SC_CTHREAD(threadB, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<unsigned> c{"c"};
    unsigned k;
    
    void methB() {
        mif_aa.a = 1;
        mif_a[c.read()].a = 2;
    }
    
    void threadB() {
        mif_aa.a   = 3;
        mif_aa.m   = 3;
        mif_a[0].a = 3;
        mif_a[0].m = 3;
        wait();
        
        while (true) {
            unsigned l = mif_aa.a + mif_aa.m + mif_a[0].a + mif_a[0].m;
            mif_aa.a = 4;
            mif_aa.m = 4;
            mif_a[c.read()].a = 4;
            mif_a[c.read()].m = 4;
            wait();
        }
    }
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned M = 2;
    
    sc_vector<B> mif_b{"mif_b", M};
    
    sc_vector<sc_signal<bool>> req{"req", M};
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < M; ++i) {
            mif_b[i].clk(clk); mif_b[i].rstn(rstn);
            mif_b[i].req(req[i]);
        }
      
        SC_METHOD(mainMeth);
        sensitive << s;
        
        SC_CTHREAD(mainThread, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<16>> s{"s"};
    
     void mainMeth() {
        mif_b[s.read()].c = 1;
        mif_b[s.read()].k = 1;
    }
    
    void mainThread() {
        mif_b[s.read()].mif_aa.b = 1;
        mif_b[s.read()].mif_aa.n = 1;
        wait();
        
        while (true) {
            unsigned l = mif_b[0].mif_aa.b + mif_b[1].mif_aa.b +
                         mif_b[1].mif_a[0].b + mif_b[0].mif_a[1].n;
            
            mif_b[s.read()].mif_aa.b = 1;
            mif_b[s.read()].mif_aa.n = 1;

            mif_b[s.read()+1].mif_a[s.read()].b = 2;
            mif_b[s.read()+1].mif_a[s.read()].n = mif_b[s.read()].mif_aa.n;
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
