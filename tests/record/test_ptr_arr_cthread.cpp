/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Record pointer array in CTHREAD
template <unsigned N>
class A : public sc_module {
public:
    sc_in_clk       clk;
    sc_signal<bool> rst;
    
    sc_signal<int>  s;

    struct Simple {
        bool a;
        sc_uint<4> b;
        bool getA() {return a;}
        void setB(sc_uint<4> par) {b = par;}
    };
    
    Simple* r1[N];
    Simple* r2[N];
    Simple* r3[N];
    
    SC_CTOR(A) 
    {
        for (int i = 0; i < N; i++) {
            r1[i] = sc_new<Simple>();
            r2[i] = sc_new<Simple>();
            r3[i] = sc_new<Simple>();
        }
        
        SC_CTHREAD(rec_ptr_simple, clk.pos());  
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(rec_ptr_loop, clk.pos());  
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(rec_ptr_unknw, clk.pos());  
        async_reset_signal_is(rst, 0);
    }
    
    void rec_ptr_simple() 
    {
        wait();
        while(true) 
        {
            bool b = r1[0]->a;
            b = r1[s.read()]->getA();
            wait();
        }
    }
    
    void rec_ptr_loop() 
    {
        wait();
        while(true) 
        {
            bool b = false;

            for (int i = 0; i < N; i++) {
                b = b || r2[i]->getA();
                
                if (!b) r2[i]->setB(i);
            }
            wait();
        }
    }
    
    void rec_ptr_unknw() 
    {
        wait();
        while(true) 
        {
            int i = s;
            bool b = r3[i]->a;
            sc_uint<4> c = r3[i+1]->b;
            r3[i]->setB(i+2);
            wait();
        }
    }  
};

class B_top : public sc_module {
public:
    sc_in_clk       clk;
    A<2> a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    B_top b_mod{"b_mod"};
    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

