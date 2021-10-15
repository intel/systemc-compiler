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

// Duplicate variable declaration for variable used in multiple processes
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk               clk;
    sc_in<bool>             rst;

    sc_in<int>*             in;     
    sc_out<int>*            out;
    sc_signal<int>*         p;
    sc_uint<4>*             vp;

    SC_CTOR(mod_if) 
    {
        in = new sc_in<int>("in");
        out = new sc_out<int>("out");
        p = new sc_signal<int>("p");
        vp = sc_new<sc_uint<4>>();
        
        SC_METHOD(ptrProc);
        sensitive << *p << *in;

        SC_CTHREAD(thrProc, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(thrSecProc, clk.pos());
        async_reset_signal_is(rst, true);
    }

    void ptrProc() {
        *p = in->read();
        *vp = 3;
    }
    
    // TODO: fix multiple declaration of @vp
    void thrProc() {
        *p = 4;
        wait();
        
        while (true) {
            *vp = 5;
            wait();
        }
    }

    // TODO: fix multiple declaration of @vp
    void thrSecProc() {
        *p = 6;
        wait();
        
        while (true) {
            *vp = 7;
            wait();
        }
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  t;      // Not used therefore not assigned
    sc_signal<int>  z;      // Not used therefore not assigned
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
        minst[1]->out->bind(t);
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
