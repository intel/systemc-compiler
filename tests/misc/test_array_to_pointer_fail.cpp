/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Pointer to non-zero array element not supported yet -- error reported
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    int*          pp;
    int           pa[3];
    
    sc_signal<int> s;

    SC_CTOR(A)
    {
        pp = &pa[1];
        
        SC_METHOD(read_pointer); 
        sensitive << s;
    }

    void read_pointer()
    {
        int i;
        i = pp[1];
    }
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk("clk", 1, SC_NS);
    A a_mod{"b_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}

