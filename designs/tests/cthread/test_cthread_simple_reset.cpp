/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Empty CTHREAD with async reset signal and signal pointer
class A : public sc_module {
public:

    sc_in<bool>         clk{"clk"};
    
    sc_signal<bool>     rst_sig;
    sc_signal<bool>*    rst_sig_p;
    
    SC_CTOR(A) {
        rst_sig_p = new sc_signal<bool>("rst_sig_p");
        
        SC_CTHREAD(simple_rst, clk.pos());
        async_reset_signal_is(rst_sig, false);

        SC_CTHREAD(simple_rst_p, clk.pos());
        async_reset_signal_is(*rst_sig_p, false);
    }
    
    
    void simple_rst() {
        wait();
        
        while (true) {
            wait();
        }
    }
    
    void simple_rst_p() {
        wait();
        
        while (true) {
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk, 1, SC_NS"};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

