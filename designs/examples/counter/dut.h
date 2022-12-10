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
    sc_out<sc_uint<4> > counter{"counter"};
    sc_out<bool>        even{"even"};
 
    SC_CTOR(Dut) 
    {
	SC_CTHREAD(threadProc, clk.pos());
	async_reset_signal_is(rstn, false);

	SC_METHOD(methodProc);
	sensitive << counter;
    }

    void threadProc() {
        counter = 0;
        wait();
        while(1) {
            counter = counter.read() + 1;
            wait();
        }
    }

    void methodProc() {
        even = counter.read() % 2;
    }

};