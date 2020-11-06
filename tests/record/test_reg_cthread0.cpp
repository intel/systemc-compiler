/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Local records in thread with field used as registers
struct SinCosTuple 
{
    int sin = 1;
    int cos = 2;
};

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    sc_signal<int> s;
    
    SC_CTOR(A)
    {
        SC_CTHREAD(record_ro, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(record_comb, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_reg1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_reg2, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    // Local record read-only defined in reset section
    void record_ro() {
        const SinCosTuple r;
        const int a = 1;
        wait();
        
        while (true) {
            int b = a; 
            b = r.sin;
            wait();
        }
    }
    
    // Local record comb
    void record_comb() {
        wait();
        
        while (true) {
            SinCosTuple r;
            r.sin = 3;
            int b = r.sin;
            wait();
        }
    }
    
    // Local record register
    void record_reg() {
        wait();
        
        while (true) {
            SinCosTuple r;
            wait();
            int b = r.sin;
            r.sin = 3;
        }
    }
    
    // Local record register declared in reset
    void record_reg1() {
        
        SinCosTuple r;
        wait();
        
        while (true) {
            int b = r.sin;
            wait();
        }
    }
    
    // Two local record registers
    void record_reg2() {
        
        SinCosTuple r;
        SinCosTuple s;
        wait();
        
        while (true) {
            int b = r.sin + s.cos;
            wait();
        }
    }    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    
    sc_start();
    return 0;
}

