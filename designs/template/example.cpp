/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

// Design template

#include "dut.h"
#include <systemc.h>
 
// ICSC requires DUT top should be instantiated inside wrapper (typically TB) 
// and all DUT ports are bound.
struct Tb : sc_module 
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rstn{"rstn"};
 
    Dut dut_inst{"dut_inst"};

    SC_CTOR(Tb) 
    {
        dut_inst.clk(clk);
        dut_inst.rstn(rstn);
        
        SC_CTHREAD(test_proc, clk.pos());
    }
    
    // Assert and de-assert reset for DUT
    void test_proc() 
    {
        rstn = 0;
        wait();
        rstn = 1;
        
        // Add testbench code here
    }
};
 
int sc_main (int argc, char **argv) 
{
    sc_clock clk{"clk", sc_time(1, SC_NS)};
    Tb tb("tb");
    tb.clk(clk);
    sc_start();
    return 0;
}