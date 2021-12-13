/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Intrinsic modified in module constructor
struct A : public sc_module {
    
    sc_in<bool> clk;
    sc_signal<bool> nrst{"nrst"};
    
    sc_signal<unsigned> t;
    sc_signal<unsigned> s;
    
    static const unsigned ZERO = 0;

    SC_CTOR(A) 
    {
        SC_CTHREAD(emptyThread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(emptyMethod);
        sensitive << t;
    }
    
    void emptyThread() {
        if (ZERO) {
            t = 0;
            wait();

            while (true) {
                t = 2;    
                wait();
            }
        }
    }
    
    void emptyMethod() {
        if (ZERO) {
            s = 1;
        }
    }
    
};



int sc_main(int argc, char **argv) {
    sc_clock clk{"clk", sc_time(1, SC_NS)};
    A a("a");
    a.clk(clk);
    
    sc_start();

    return 0;
}
