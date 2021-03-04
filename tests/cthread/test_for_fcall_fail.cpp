/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// for statement with function call in condition and initialization, error reported
class A : public sc_module 
{
public:
    sc_in_clk clk;
    sc_signal<bool>     nrst;
    sc_signal<bool>     s;
    
    SC_CTOR(A) {
        SC_CTHREAD(func_call_cond, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(func_call_init, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    // For with function call in condition w/o comparison
    bool cond(int i) {
        return (i < 2);
    }
    
    void func_call_cond() {
        wait();
        while(true) {
            int i = s.read();
            for (; cond(i); i++) {
                wait();
            }
            i = 0;
            wait();
        }
    }
    
    int init1() {
        return 3;
    }
    void func_call_init() {
        int k = 0;
        for (int i = init1(); i < 10; i++) {
            k++;
        }
    }
};



int  sc_main(int argc, char* argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    A a{"a"};
    a.clk(clk);
    sc_start();
    return 0;
}

