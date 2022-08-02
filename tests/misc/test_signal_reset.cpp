/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Reset is sc_signal asserted by one of the processes
SC_MODULE(B) {
    sc_in<bool>     clk;
    sc_in<bool>     rstn;
    
    SC_CTOR(B) {
        SC_METHOD(methodProc);
        sensitive << rstn;

        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
    }    
    
    sc_signal<int> s;
    void methodProc() {
        if (rstn) {
            s = 0;
    	} else {
            s = 1;
        }
    }
    
    sc_signal<int> t;
    void threadProc() {
        t = 0;
        wait();
        while (true) {
            t = 1;
            wait();
        }
    }
};

SC_MODULE(A) {
    sc_in<bool>      clk;
    sc_signal<bool>  rstn;
    
    B b_mod{"b_mod"};
    
    SC_CTOR(A) {
        b_mod.clk(clk);
        b_mod.rstn(rstn);

        SC_CTHREAD(resetProc, clk.pos());
    }
    
    sc_uint<16> cntr;
    void resetProc() {
        while (true) {
            cntr++;
            rstn = cntr != 10;
            wait();
        }
    }
};


int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);

    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();

    return 0;
}


