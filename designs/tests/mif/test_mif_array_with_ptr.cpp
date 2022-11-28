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

// Array of modular interface pointers accessed at unknown index
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    sc_signal<sc_uint<4>>   s {"s"};
    sc_uint<4>              v;
    sc_uint<4>              vv;
    sc_signal<int>*         p;
    sc_signal<int>*         pp;
    sc_uint<4>*             vp;
    sc_uint<5>*             vvp;

    SC_CTOR(mod_if) 
    {
        p = new sc_signal<int>("p");
        pp = new sc_signal<int>("pp");
        vp = sc_new<sc_uint<4>>();
        vvp = sc_new<sc_uint<5>>();

        SC_METHOD(mif_meth);
        sensitive << s;

        SC_CTHREAD(mif_thread, clk.pos());
        async_reset_signal_is(rst, true);
    }

    void mif_meth() {
        v = s.read();
        *vp = 3;
    }
    
    void mif_thread() {
        *p = 0;
        wait();
        
        while (true) {
            *p = 1;
            wait();
        }
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  t;
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);
        }
        
        SC_METHOD(top_method);
        sensitive << minst[0]->s << minst[1]->s << t;
        
        SC_CTHREAD(top_thread, clk.pos());
        async_reset_signal_is(rst, true);
    }

    void top_method() 
    {
        int i = t;
        minst[1]->s = 1;

        minst[1]->p->write(2);
        (*minst[1]->p).write(3);
        *(minst[1]->p) = 4;
        
        *minst[1]->vvp = 5;
        
        sc_uint<4> a = minst[i]->s;
        minst[i]->s = minst[i+1]->vv + a;
    }
    
    
    void top_thread() {
        *(minst[1]->pp) = 0;
        wait();
        
        while (true) {
            *(minst[1]->pp) = 1;
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
