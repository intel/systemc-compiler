/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Code after return -- error reported
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD(meth1);
        sensitive << s;
    }
    
    sc_signal<int> s;
    
    int f(int i) {
        int k = i+1;
        if (s.read() == k) {
            return k;
        }
        k--;
        return 0;
    }

    void meth1() 
    {
        if (s.read()) {
            f(42);
        }
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

