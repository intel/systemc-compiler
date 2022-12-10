/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_comb_signal.h"
#include <systemc.h>
#include <string>

// Combinational signal test
struct comb_signal_module : public sc_module
{
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};
    sc_signal<bool>             a{"a"};
    sc_signal<bool>             b{"b"};
    sct_comb_signal<bool, 0>    req1{"req1"};
    sct_comb_signal<bool, 1>    req2{"req2"};
    
    SC_CTOR(comb_signal_module) 
    {
        SC_METHOD(methProc);
        sensitive << nrst << req1 << req2;

        SC_CTHREAD(thrdProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void methProc() {
        if (nrst) {
            b = 0;
            b = 0;
        } else {
            b = req1;
            b = req2;
        }
    }
    
    void thrdProc() {
        bool a = 0;
        req1 = 1;
        req2 = 0;
        b = 0;
        wait();             // 0
        
        while (true) {
            b = req1;
            b = req2;
            
            req1 = a;
            req2 = a;
            a = !a;
            wait();         // 1
        }
    }
};

struct testbench : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    comb_signal_module  mod{"mod"};
    
    SC_CTOR(testbench) 
    {
        mod.clk(clk);
        mod.nrst(nrst);
        SC_CTHREAD(test, clk.pos());
    }
    
    void test() {
        nrst = 0;
        wait(5); 
        
        nrst = 1;
        wait(10);   
        
        sc_stop();
    }
};


int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 1, SC_NS};
    testbench tb{"tb"};
    tb.clk(clock_gen);
    sc_start();

    return 0;
}

