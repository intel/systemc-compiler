/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Test for variable read-only after initialization in reset/previous wait()
// 1) In CTHREAD initialize variable in reset and read in inf loop body.
// 2) In CTHREAD initialize variable before wait() and read after it, no write after this wait().
class Top : sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     arstn{"arstn"};
    sc_signal<int>      in{"in"};

    SC_HAS_PROCESS(Top);
    Top(sc_module_name)
    {
        SC_CTHREAD(test_cthread1, clk);
        async_reset_signal_is(arstn, false);


        SC_CTHREAD(test_cthread2, clk);
        async_reset_signal_is(arstn, false);
    }



    void test_cthread1()
    {
        sc_uint<31> var_a = 1;
        wait();
        
        while (1) {
            in = var_a;
            wait();
        }
    }

    void test_cthread2()
    {
        wait();
        
        while (1) {
            sc_uint<31> var_a = 1;
            wait();
            in = var_a;
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

