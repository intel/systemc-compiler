/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include <systemc.h>
 
struct Dut : sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_signal<sc_uint<16>>  s{"s"};
    
    SC_CTOR(Dut) 
    {
	SC_CTHREAD(threadProc, clk.pos());
	async_reset_signal_is(rstn, false);

	SC_METHOD(methodProc);
	sensitive << s;  // Add all signal/ports read in method function here
    }

    void threadProc() 
    {
        // Reset signal/output port values here
        wait();
        
        while (true) {
            // Place sequential logic here
            wait();
        }
    }

    void methodProc() {
        // Place combinational logic here
    }

};