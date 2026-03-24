/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Register assigned and used in reset, check blocking/non-blocking assignment
// For register in @always_ff non-blocking assignment used, so registers should 
// not be read in thread reset section
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    
    sc_signal<bool> s;

    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(var_in_reset, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    

    sc_signal<int> t0;
    sc_signal<int> t1;
    void var_in_reset() 
    {
        int v = 42;         
        int w = v;          // Warning reported for using register in reset
        t0 = w;             // Warning reported for using register in reset
        wait();
        
        while (true) {
            t1 = t0;
            v = v + 1;
            t0 = v + w;
            wait();
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

