/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Loop with wait() contains combinational path through all iterations
// This test should fail
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int>  a{"a"};
    sc_signal<int>  b{"b"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(test_thread);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    void test_thread()
    {
        wait();
        
        while (1) {
            
            for (int i = 0; i < 3; i++) {
                while (a.read()) {
                    if (a.read() > b.read()) {
                        wait();
                    }
                }
            }
            
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

