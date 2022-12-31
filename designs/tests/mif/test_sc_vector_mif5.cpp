/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// sc_vector of MIF which has sc_vector of MIF inside, member accesses and function calls

struct A : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_in<bool>  req;
    sc_signal<unsigned>  resp{"resp"};
    unsigned mm = 2;
    
    SC_HAS_PROCESS(A);
    
    A(sc_module_name) 
    {
        SC_METHOD(methProcA); sensitive << req;
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void methProcA() {
        resp = 1;
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
        
        SC_METHOD(methProcB);
        for (int i = 0; i < M; ++i) sensitive << mif_a[i].resp; 
        sensitive << mif_aa.resp;
        sensitive << req;
    }
    
    sc_signal<unsigned> result{"result"};
    void methProcB() {
        result = mif_aa.resp + mif_a[0].resp;
        int j = mif_aa.resp + mif_a[0].resp;
        if (req) {
            result = mif_aa.getData() % 3;
            result = mif_a[j].getData() % 3;
        }
    }
    
    unsigned getRes() {
        return (result.read() + mif_aa.resp + mif_a[0].mm + mif_a[1].resp);
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
        for (int i = 0; i < M; ++i) {
            sensitive << mif_b[i].result << mif_b[i].mif_aa.resp;
            for (int j = 0; j < B::M; ++j) {
                sensitive << mif_b[i].mif_a[j].resp;
            }
        }
        
        SC_CTHREAD(mainThread, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<16>> s{"s"};
    
     void mainMeth() {
        unsigned l;
        l = mif_b[s.read()].getRes();
        l = mif_b[s.read()].mif_aa.getData();
    }
    
    void mainThread() {
        wait();
        
        while (true) {
            unsigned l;
            l = mif_b[s.read()].mif_aa.resp;
            l = mif_b[s.read()].mif_a[0].resp;
            l = mif_b[s.read()+1].mif_a[s.read()+2].resp;
            l = mif_b[s.read()].getRes();
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
