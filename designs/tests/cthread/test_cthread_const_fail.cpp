/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

// Constant modified in process, error reported
template<unsigned N>
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    const unsigned  C5 = 66;
    const unsigned  C6 = 67;
    
    const unsigned* p5;
    unsigned* p6;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        p5 = &C5;
        p6 = const_cast<unsigned*>(&C6);
        
        SC_CTHREAD(const_with_ptr_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
    }
    
    sc_signal<unsigned> s;
    void const_with_ptr_thread() 
    {
        unsigned j = C5 + C6;
        wait();
        
        while (true) {
            
            j = *p5;
            j = *p6;
            *p6 = 1;
            s = C6;
            
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    A<2> a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

