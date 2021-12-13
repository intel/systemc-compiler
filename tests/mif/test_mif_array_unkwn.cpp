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
    sc_signal<sc_uint<4>>   s {"s"};
    sc_uint<4>              v;
    sc_uint<4>              vv;
    sc_signal<int>*         p;
    sc_uint<4>*             vp;

    SC_CTOR(mod_if) 
    {
        p = new sc_signal<int>("p");
        vp = sc_new<sc_uint<4>>();

        SC_METHOD(metProc);
        sensitive << s;

        //SC_CTHREAD(thrProc, clk.pos());
        //async_reset_signal_is(rst, true);
    }

    void metProc() {
        //v = s.read();
        s = 1;
        *p = 2;
        *vp = 3;
    }
    
    void f() {
        v = s;
    }
    
//    void thrProc() {
//        v = 1;
//        wait();
//        while (true) {
//            bool b = v++;
//            wait();
//        }
//    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  t;
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
        }
        
        SC_METHOD(top_method);
        sensitive << minst[0]->s << minst[1]->s << t;
    }

    void top_method() 
    {
        int i = t;
        minst[i]->s = 1;

        minst[i]->p->write(2);
        (*minst[i]->p).write(3);
        *(minst[i]->p) = 4;
        
        *minst[i]->vp = 5;
        
        minst[i]->vv = 0;
        for (int j = 0; j < 2; ++j) minst[j]->v = j;

        sc_uint<4> a = minst[i]->s;
        minst[i]->s = minst[i+1]->v + a;
        
        minst[i]->f();
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
