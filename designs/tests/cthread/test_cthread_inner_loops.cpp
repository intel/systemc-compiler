/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Inner loops with multiple wait()
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<bool> in_bool{"in_bool"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(test_thread1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }
    
    void test_thread1()
    {
        wait();
        
        while (1) {
            for (size_t i = 0; i < 3; ++i) { 
                while (in.read() > 10) {     
                    wait();         // 1
                }                   
                wait();             // 2
            }                       
        }
    }

    sc_signal<int> out{"out"};
    void test_thread2()
    {
        out = 0;
        wait();
        
        while (1) {
            for (size_t i = 0; i < 3; ++i) {
                while (in.read() > 10) {
                    out = 1;
                    if (in_bool.read()) {
                        wait();     // 1
                        out = 2;
                    }
                    wait();         // 2
                    out = 3;
                }
                wait();             // 3
                out = 4;
            }
        }
    }
    
    sc_signal<int> out2{"out2"};
    void test_thread3()
    {
        out2 = 0;
        wait();
        
        while (1) {
            for (size_t i = 0; i < 3; ++i) {
                for (size_t j = 0; j < 4; ++j) {
                    while (in.read() > 10) {
                        out2 = i+j;
                        wait(); // 1
                    }
                    wait();     // 2
                }
                if (in_bool.read()) {
                    wait();     // 3
                    out2 = 2;
                }
                wait();         // 4
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

