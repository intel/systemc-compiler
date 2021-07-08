/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Function call in reset, functions with wait()
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

        SC_CTHREAD(top_thread3, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(top_thread4, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(top_thread5, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    int f() {
        return 42;
    }
    
    int g(int i) {
        i++;
        const int F = 42*i;
        return (F+1);
    }

    // Constant in function, @F remains local constant
    sc_signal<int> s0;
    void top_thread0() 
    {
        int i = g(11);
        s0 = i;
        wait();
        
        while(true) 
        {
            wait();
        }
    }

    sc_signal<int> s1;
    void top_thread1() 
    {
        const int c = f();
        static const int cs = f()-1;

        int k = c % cs;
        wait();
        
        while(true) 
        {
            int j = c / cs + k;
            s1 = j;
            wait();
        }
    }
    
    sc_signal<int> s2;
    void top_thread2() 
    {
        int i = f();
        const int c = 41+i;
        static const int cs = 42;

        wait();
        
        while(true) 
        {
            int j = c + i;
            wait();
            j += cs;
            s2 = j;
        }
    }
    

    void fr(int& par) {
        par--;
    }

    sc_signal<int> s3;
    void top_thread3() 
    {
        int i = f();
        fr(i);
        s3 = i;
        wait();
        
        while(true) 
        {
            wait();
        }
    }
    
// ----------------------------------------------------------------------------

    int fw(const int& par) {
        int l = par + par;
        wait();
        return (l - par);
    }

    sc_signal<int> s4;
    void top_thread4() 
    {
        int i = 10;
        s4 = fw(i);
        wait();
        
        while(true) 
        {
            wait();
        }
    }    
    
    sc_signal<int> s5;
    void top_thread5() 
    {
        s5 = fw(11);
        wait();
        
        while(true) 
        {
            wait();
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
