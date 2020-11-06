/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Function call in reset with literal parameter 
SC_MODULE(Top) 
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst;

    sc_signal<int>      s;
    
    SC_CTOR(Top)
    {
        SC_CTHREAD(top_thread0, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(top_thread1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(top_thread2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    int f() {
        return 42;
    }
    
    int g() {
        const int F = s.read();
        return (F+1);
    }

    // Constant in function, @F remains local constant
    void top_thread0() 
    {
        int i = g();
        wait();
        
        while(true) 
        {
            wait();
        }
    }

    void top_thread1() 
    {
        const int c = f();
        static const int cs = f()-1;

        int k = c % cs;
        wait();
        
        while(true) 
        {
            int j = c / cs + k;
            wait();
        }
    }
    
    void top_thread2() 
    {
        int i = f();
        const int c = s.read();
        static const int cs = 42;

        wait();
        
        while(true) 
        {
            int j = c + i;
            wait();
            j += cs;
        }
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
