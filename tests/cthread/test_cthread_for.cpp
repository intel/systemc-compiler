/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/13/18.
//

#include <systemc.h>

class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(for_stmt_no_wait);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(for_stmt_wait0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(for_stmt_wait1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(for_stmt_wait2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(for_stmt_wait_noiter);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(thread_break);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    // ------------------------------------------------------------------------
    // For DAC paper
    sc_signal<int> enabled{"enabled"};
    sc_signal<int> reset{"reset"};
    
    void thread_break() {
        wait();          // STATE 0
        while (true) {
            wait();      // STATE 1
            while (!enabled) {
                if (reset) break;
                wait();  // STATE 2
            }
        }
    }

    // ------------------------------------------------------------------------
    void for_stmt_no_wait()
    {
        int k = 0;
        wait();
        
        while (true) {
            
            for (int i = 0; i < 2; i++) {
                k = 1;
            }
            
            k = 2;
            wait();
        }
    }
    
    void for_stmt_wait0()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;                          // B6
            wait();     // 1
            
            for (int i = 0; i < 2; i++) {   // B5
                k = 2;                      // B4
                wait();   // 2
            }                               // B3
            k = 3;                          // B2
        }
    }
    
    void for_stmt_wait1()
    {
        int k = 0;
        wait();
        
        while (true) {
            for (int i = 0; i < 2; i++) {
                k = 1;
                wait();
            }
            k = 2;
        }
    }

    void for_stmt_wait2()
    {
        int k = 0;
        wait();
        
        while (true) {
            for (int i = 0; i < 2; i++) {
                k = 1;
                wait();
            }
            k = 2;
            wait();
            
            k = 3;
        }
    }
    
    // For with wait() no iteration
    void for_stmt_wait_noiter()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;
            for (int i = 0; i < 0; i++) {
                k = 2;
                wait();
            }
            k = 3;
            wait();
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

