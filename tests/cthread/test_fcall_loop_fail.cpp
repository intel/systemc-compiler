/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// FOR statement with function call in increment and initialization, error reported
// Function call in loop condition with wait(), error reported
class A : public sc_module 
{
public:
    sc_in_clk clk;
    sc_signal<bool>     nrst;
    sc_signal<bool>     s;
    
    SC_CTOR(A) {
        //SC_METHOD(meth1); sensitive << s;

        //SC_CTHREAD(func_call_incr, clk.pos());
        //async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(func_call_init, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(func_call_cond_wait1, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void meth1() {
        int k = s.read();
        for (int i = 0; i < incr1(3); i++) {
            k++;
            wait();
        }
    }
    
    // For with function call in condition w/o comparison
    bool incr1(int i) {
        return (i+1);
    }
    
    void func_call_incr() {
        wait();
        while(true) {
            int i = s.read();
            for (; i < 10; i = incr1(i)) {
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
        wait();
        while(true) {
            int k = 0;
            for (int i = init1(); i < 10; i++) {
                k++;
                wait();
            }
            wait();
        }
    }
    
    
     int wait1(int i) {
        int j = i+1;
        wait();
        return j;
    }
    void func_call_cond_wait1() {
        wait();
        while(true) {
            for (int i = 0; i < wait1(i); i++) {
                wait();
            }
            wait();
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

