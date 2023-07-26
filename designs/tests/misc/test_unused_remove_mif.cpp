/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

// Check unused variables/statements in MIF removed
struct A : public sc_module, sc_interface
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;
    sc_signal<int>      t0;
    sc_signal<int>      t1;
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(removeMeth); sensitive << s;
        
        SC_CTHREAD(removeThrd, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    int m1;         // removed
    int m2;
    void removeMeth() 
    {
        int x; x = 1;                   // removed
        sc_uint<4> y; y = s.read(); 
        
        int xn;                         // removed
        sc_uint<4> yn = y + 1;
        t0 = yn;

        int i = 1;
        int j = i + 1;
        t0 = j;
        
        m1 = 42;                        // removed
        m2 = 42;
        t0.write(m2 + 1);
    }    
    
    
    int m3;
    int m4;         // removed
    int m5;         // removed
    void removeThrd()
    {
        int i;                          // removed
        t1 = 0; m3 = 0;
        wait();

        while (true) {
            m3 = m3 + 1;
            t1 = m3;
            m4 = t1;
            i = m5;
            wait();
        }
    }        
};

struct B : public sc_module
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_vector<A>        a{"a", 2};
    
    sc_signal<int>      t2;
    sc_signal<int>      t3;
    
    B(const sc_module_name& name) : sc_module(name) 
    {
        for (int i = 0; i < 2; ++i) {
            a[i].clk(clk);
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    B b_mod{"b_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

