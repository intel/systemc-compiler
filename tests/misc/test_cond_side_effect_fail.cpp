/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

// Check unused variables/statements remove
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        
        SC_METHOD(meth1); sensitive << s;
        SC_METHOD(meth2); sensitive << s;
    }
    
    sc_signal<int> t1;
    void meth1() 
    {
        int i = 1;
        if (i++) {     // true condition
            t1 = i;
        }
        int j;
        if (j = i) {   // true condition
            t1 = j;
        }
        sct_assert(i == 2);
    }
    
    sc_signal<int> t2;
    void meth2() 
    {
        int i = 1;
        const int j = i++;    // @j is not replaced -- OK
        t2 = j;
        sct_assert(i == 2);
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

