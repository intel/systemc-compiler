/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by Mikhail Moiseev
//

#include "sct_assert.h"
#include <systemc.h>

// Channel read in CTHREAD reset -- warning reported
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> arstn;
    
    sc_in<bool> a;
    sc_out<bool> b;
    sc_signal<sc_uint<4>> s[3];
    sc_signal<sc_uint<4>> t[3];
    sc_vector<sc_signal<int>> v{"v", 3};

    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(incr_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);
    }
    
    // Increment/decrement in reset
    void incr_in_reset() 
    {
        const int C = 2;
        int i = 1;
        bool x = a.read();         
        sc_uint<4> j = s[0];
        j = 2 * t[2].read();
        j = b.read() ? i : v[C].read();
        
        wait();
        
        while (true) {
            wait();
        }
    }
    
};

int sc_main(int argc, char *argv[])
{
    sc_signal<bool> a;
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.a(a);
    top_inst.b(a);
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

