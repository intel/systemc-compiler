/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Check unused register declaration and initialization in reset removed, #314 
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(methProc);
        sensitive << t;
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<sc_uint<4>> r;
    sc_signal<sc_uint<4>> s;
    sc_signal<sc_uint<4>> t;

    unsigned m;
    
    void methProc() 
    {
        r = 0;          // Not used -- not removed as it is too difficult, see #314
    }
    
    void threadProc() 
    {
        bool a = 0;     // Not used
        m = 42;         // Not used    
        s = 0;          // Not used -- not removed as it is too difficult, see #314
        t = 0;
        wait();
        
        while (true) {
            
            t = t.read() + 1;
            
            wait();

        }
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);

    sc_start();
    return 0;
}

