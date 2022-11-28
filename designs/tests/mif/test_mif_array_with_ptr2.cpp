/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 06/14/19.
//

#include <systemc.h>

// Array of modular interface pointers with port/signal pointers
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk               clk;
    sc_in<bool>             rst;
    
    sc_in<int>*             in;
    sc_out<int>*            out;
    sc_signal<int>*         p;
    sc_signal<int>*         q;
    sc_uint<4>*             vp;

    SC_CTOR(mod_if) 
    {
        in = new sc_in<int>("in");
        out = new sc_out<int>("out");
        p = new sc_signal<int>("p");
        q = new sc_signal<int>("q");
        vp = sc_new<sc_uint<4>>();
        
        SC_METHOD(ptrProc);
        sensitive << *p << *in;

        SC_CTHREAD(thrProc, clk.pos());
        async_reset_signal_is(rst, true);
    }

    void ptrProc() {
        *out = *in;
        *p = 2;
        *vp = 3;
    }
    
    void thrProc() {
        *p = 2;
        wait();
        
        while (true) {
            *out = 1;
            p->write(in->read());
            wait();
        }
    }

    int getP() {
        return p->read();
    }

    void setQ(int val) {
        *q = val;
    }
    
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  t;
    sc_signal<int>  z;
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);
        }
        minst[0]->in->bind(t);
        minst[1]->in->bind(z);
        minst[0]->out->bind(t);
        minst[1]->out->bind(z);
        
        SC_METHOD(top_method);
        sensitive << t << *minst[0]->p;
        
        SC_CTHREAD(top_thread, clk.pos());
        async_reset_signal_is(rst, true);
    }

    void top_method() 
    {
        int j = 0;
        for (int i = 0; i < 2; i++) {
            j += minst[i]->getP();
        }
    }

    void top_thread() 
    {
        int j = 0;
        for (int i = 0; i < 2; i++) {
            j += minst[i]->getP();
        }
        wait();
        
        while(true) {
            
            for (int i = 0; i < 2; i++) {
                minst[i]->setQ(i);
            }
            wait();
        }
        
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
