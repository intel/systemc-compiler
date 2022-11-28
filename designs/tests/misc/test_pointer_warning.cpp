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

// Check no extra warning for read/de-reference pointer
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;

    int* q0{sc_new<int>()};
    int* q1{sc_new<int>()};
    sc_uint<4>* p0;
    sc_uint<4>* p1;
    sc_uint<4> v0;
    sc_uint<4> v1;
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        p0 = &v0; 
        p1 = &v1;
        SC_METHOD(nonconst_ptr_method);
        sensitive << s;

        SC_CTHREAD(nonconst_ptr_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    void nonconst_ptr_method() {
        *q0 = 1; v0 = 2;
        int j = *q0 + *p0;
    }
    
    void nonconst_ptr_thread() {
        *q1 = 1; v1 = 2;
        wait();
        
        while (true) {
            auto lp = q1; 
            int j = *q1 + *p1 + *lp;
            wait();
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

