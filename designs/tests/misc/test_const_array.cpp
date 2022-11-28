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

// Constant boolean and array of boolean 
template<unsigned N>
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;

    const bool  C1 = true;
    const bool  C2 = false;
    const bool  arr[3] = {true, true, false};

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD(array_method);
        sensitive << s;
    }
    
    void array_method() 
    {
        const bool larr[3] = {true, true, false};
        
        bool b1 = arr[1];
        b1 = larr[1];
        bool b2 = C1;
        b2 = C2;
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

