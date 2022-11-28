/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// for loop with SC integer counter
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(for_cntr_scint);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(for_cntr_scint2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }
    
    static const unsigned TIME_CNTR_WIDTH = 5;
    
    sc_signal<sc_uint<TIME_CNTR_WIDTH> > wakeup_time;  
    
    void for_cntr_scint()
    {
        wait();
        
        while (true) {
            wait();         // 1
            
            sc_uint<TIME_CNTR_WIDTH> wakeupTime = wakeup_time;
            for (sc_uint<TIME_CNTR_WIDTH> i = 0; i < wakeupTime; i++) {
                wait();     // 2
            }
            
        }
    }
    
    void for_cntr_scint2()
    {
        wait();
        
        while (true) {
            wait();         // 1
            
            unsigned wakeupTime = wakeup_time.read();
            for (sc_int<TIME_CNTR_WIDTH> i = 0; i < wakeupTime; i++) {
                wait();     // 2
            }
            
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

