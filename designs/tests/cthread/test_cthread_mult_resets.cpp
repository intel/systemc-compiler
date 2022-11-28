/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Multiple sync and async resets for CTHREAD
class top : sc_module
{
public:
    sc_in<bool> clk;
    sc_in<bool> arstn1;
    sc_in<bool> arstn2;
    sc_in<bool> sreset1;
    sc_in<bool> sreset2;
    sc_in<bool> sreset3;

    sc_signal<int> in{"in"};

    SC_HAS_PROCESS(top);

    top(sc_module_name)
    {
        SC_CTHREAD(test_thread1, clk.pos());
        async_reset_signal_is(arstn1, false);
        async_reset_signal_is(arstn2, true);
        reset_signal_is(sreset1, true);

        SC_CTHREAD(test_thread2, clk.pos());
        async_reset_signal_is(arstn1, false);
        reset_signal_is(sreset1, true);
        reset_signal_is(sreset2, true);
        reset_signal_is(sreset3, true);
    }

    sc_signal<int> out;
    void test_thread1()
    {
        out = 0;
        while (1) {
            wait();
            out = 1;
        }
    }

    sc_signal<int> out2;
    void test_thread2()
    {
        int i = 0;
        wait();
        
        while (1) {
            out2 = i++;
            wait();
            if (in) i--;
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> s1;
    sc_signal<bool> s2;
    sc_signal<bool> s3;
    sc_signal<bool> s4;
    sc_signal<bool> s5;

    top top_inst{"top_inst"};
    top_inst.clk(clk);
    top_inst.arstn1(s1);
    top_inst.arstn2(s2);
    top_inst.sreset1(s3);
    top_inst.sreset2(s4);
    top_inst.sreset3(s5);
    
    sc_start(100, SC_NS);
    return 0;
}

