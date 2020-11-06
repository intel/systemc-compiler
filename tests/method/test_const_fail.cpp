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

// Constant modified in METHOD
template<unsigned N>
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;
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
        
        SC_METHOD(const_with_ptr_method);
        sensitive << s;
    }
   
    void const_with_ptr_method() 
    {
        unsigned j = C5 + C6;
        j = *p5;
        j = *p6;
        *p6 = 1;
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

