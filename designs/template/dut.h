/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include <systemc.h>
 
struct Dut : sc_module 
{
    typedef sc_uint<16> data_t;
    
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_in<data_t>       inp{"inp"};
    sc_out<data_t>      outp{"outp"};
    
    
    SC_CTOR(Dut) 
    {
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_METHOD(methodProc);
        sensitive << tmps;  // Add all signal/ports read in method function here
    }
    
    sc_signal<data_t>   tmps{"tmps"};

    void threadProc() 
    {
        // Reset signal/output port values here
        tmps = 0;
        wait();
        
        while (true) {
            // Place sequential logic here
            tmps = inp;
            wait();
        }
    }

    void methodProc() {
        // Place combinational logic here
        outp = tmps.read() + data_t(1);
    }

};
