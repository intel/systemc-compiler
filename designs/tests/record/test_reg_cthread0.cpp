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
    sc_signal<int> t0;
    void record_ro() {
        const SinCosTuple r;
        const int a = 1;
        wait();
        
        while (true) {
            int b = a; 
            b = r.sin;
            t0 = b;
            wait();
        }
    }
    
    // Local record comb
    sc_signal<int> t1;
    void record_comb() {
        wait();
        
        while (true) {
            SinCosTuple r;
            r.sin = 3;
            int b = r.sin;
            t1 = b;
            wait();
        }
    }
    
    // Local record register
    sc_signal<int> t2;
    void record_reg() {
        wait();
        
        while (true) {
            SinCosTuple r;
            wait();
            int b = r.sin;
            r.sin = 3;
            t2 = r.sin;
        }
    }
    
    // Local record register declared in reset
    sc_signal<int> t3;
    void record_reg1() {
        SinCosTuple r;
        wait();
        
        while (true) {
            int b = r.sin;
            t3 = b;
            wait();
        }
    }
    
    // Two local record registers
    sc_signal<int> t4;        
    void record_reg2() {
        
        SinCosTuple r;
        SinCosTuple s;
        wait();
        
        while (true) {
            int b = r.sin + s.cos;
            t4 = b;
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

