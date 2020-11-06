/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// FOR in IF tests
class A : public sc_module {
public:

    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};
    
    SC_CTOR(A) {
        SC_HAS_PROCESS(A);
        
        SC_CTHREAD(for_in_if1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(for_in_if2, clk.pos());
        async_reset_signal_is(rst, true);
    }
    
    static const unsigned BLOCK_NUM = 3;
    static const unsigned TIME_CNTR_WIDTH = 3;
    
    sc_signal<bool>     sleep_enable{"sleep_enable"};
    sc_signal<bool>     block_access[BLOCK_NUM];
    sc_signal<bool>     mem_active[BLOCK_NUM];
    
    sc_signal<sc_uint<TIME_CNTR_WIDTH> >    sleep_time;
    sc_signal<sc_uint<TIME_CNTR_WIDTH> >    wakeup_time;
    
    void for_in_if1() 
    {
        int k = 0;
        unsigned sleepTime;
        wait();
        
        while (true) {
            sleepTime = sleep_time.read();
            if (sleep_enable)
            {                                               // B7
                for (unsigned i = 0; i < sleepTime; i++) {  // B6
                    wait();     // B5
                }               // B4
                k = 1;      // B3
            }
            k = 2;      // B2
            
            wait();
        }
    }
    
    // Loop with wait(), BUG in real design -- fixed
    void for_in_if2() 
    {
        int k = 0;
        wait();
        
        while (true) {
            bool blockFlags_flat[BLOCK_NUM];
            
            sc_uint<TIME_CNTR_WIDTH> wakeupTime = wakeup_time;
            for (unsigned i = 0; i < wakeupTime; i++) {
                wait();     
            }
            
            if (sleep_enable)
            {
                for (int i = 0; i < BLOCK_NUM; i++) {
                    blockFlags_flat[i] = block_access[i];
                    if (blockFlags_flat[i]) mem_active[i] = 0;
                }

                sc_uint<TIME_CNTR_WIDTH> sleepTime = sleep_time;
                for (unsigned i = 0; i < sleepTime; i++) {
                    wait();
                }
                k = 1;
            }
            k = 2;
            
            wait();
        }
    }
};

class B_top : public sc_module {
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    sc_clock clk_gen{"clk", sc_time(1, SC_NS)};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        clk (clk_gen);
        a_mod.clk(clk);
        a_mod.rst(rst);
        
        //SC_HAS_PROCESS(B_top);
        //SC_CTHREAD(reset_proc);
    }
    
    /*void reset_proc() {
        rst = 1;
        wait();
        
        rst = 0;
    }*/
};

int sc_main(int argc, char *argv[]) {

    B_top b_mod{"b_mod"};
//    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

