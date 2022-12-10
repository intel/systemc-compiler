/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

// Design template

#include <systemc.h>
#include "dut.h"
 
// ICSC requires DUT top should be instantiated inside wrapper (typically TB) 
// and all DUT ports are bound.
struct Tb : sc_module 
{
    typedef Dut::data_t data_t;
    
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rstn{"rstn"};
    
    sc_signal<data_t>   inp{"inp"};
    sc_signal<data_t>   outp{"outp"};
 
    Dut dut_inst{"dut_inst"};


    SC_CTOR(Tb) 
    {
        dut_inst.clk(clk);
        dut_inst.rstn(rstn);
        dut_inst.inp(inp);
        dut_inst.outp(outp);
        
        SC_CTHREAD(test_proc, clk.pos());
    }
    
    void test_proc() 
    {
        cout << "test_proc() started" << endl;
        
        // Assert and de-assert reset for DUT
        rstn = 0;
        inp = 0;
        wait();
        rstn = 1;
        
        // Add testbench code here
        inp  = 10;
        wait(2);
        inp  = 0;
        sc_assert(outp.read() == 11);
        sc_stop();
        cout << "test_proc() finished!" << endl;
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
