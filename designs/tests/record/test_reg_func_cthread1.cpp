/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Local record declared in function which called twice from thread 
// with field used as registers
struct SinCosTuple 
{
    int sin = 1;
    int cos = 2;
};

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SC_CTOR(A)
    {
        SC_CTHREAD(record_reg, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void f(int val) 
    {
        SinCosTuple r;
        int i = r.sin+val;
    }
    
    void record_reg() 
    {
        wait();
        while (true) {
            f(1);
            f(2);
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

