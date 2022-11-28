/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Multiple wait() before main loop in CTHREAD
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<sc_uint<31>> a{"a"};
    sc_signal<sc_uint<31>> b{"b"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(wait_in_reset1, clk);
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(wait_in_reset2, clk);
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(wait_in_reset3, clk);
        async_reset_signal_is(arstn, false);
    }

    void wait_in_reset1() 
    {
        a = 0;
        b = 0;
        wait();
        a = 1;
        b = 1;
        wait();
        a = 2;
        b = 3;
        wait();
        
        while (true) 
        {
            a = a.read() + 1;
            wait();
            b = a.read() + b.read();
            wait();
        }
    }
    
    sc_signal<int> s0;
    void wait_in_reset2() 
    {
        int i = 1;
        sc_uint<4> x[3];
        wait();

        x[1] = i++;
        wait();

        x[2] = i++;
        sct_assert_const(i == 3);
        sct_assert_const(x[0] == 0 && x[1] == 1 && x[2] == 2);
        wait();
        
        while (true) 
        {
            s0 = i++;
            wait();
        }
    }
    
    sc_signal<int> s1;
    void wait_in_reset3() 
    {
        // Memory initialization
        sc_uint<4> x[3];
        for (int i = 0; i < 3; ++i) {
            x[i] = i;
            wait();
        }
        int k = 0;
        wait();

        while (true) 
        {
            s1 = x[k];
            k = (k == 2) ? 0 : k+1;
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

