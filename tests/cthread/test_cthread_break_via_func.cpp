/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// break statement in loop in called function
class A : public sc_module 
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;
    
    int                 m;
    int                 k;
    int                 k1;
    int                 k2;
    int                 k3;
    int                 k4;
    int                 n;
    int*                q;
    
    SC_CTOR(A) {
        SC_CTHREAD(break_via_func1, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(break_via_func2, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(break_via_func3, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(break_via_func4, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(break_via_func5, clk.pos());
        async_reset_signal_is(nrst, false);
        
        SC_CTHREAD(fcall_break1, clk.pos());
        async_reset_signal_is(nrst, false);
    }
    
    
    void f() 
    {
        for (int i = 0; i < 2; i++) {
            if (a.read()) break;
            wait();   // 2
        }
    }
    
    void g() 
    {
        if (a) {
            int kk = 3;
        }
    }

    // @break in function, BUG in real design
    void break_via_func1() 
    {
        wait(); 
        
        while (1) {
            
            k1 = 1;
            f();
            k1 = 2;
            sct_assert_level(1);
            
            wait();  // 3
        }
    }

    void break_via_func2() 
    {
        wait(); 
        
        while (1) {
            
            k2 = 1;
            f();
            k2 = 2;
            g();
            sct_assert_level(1);

            wait();  // 3   
        }
    }

    void break_via_func3() 
    {
        wait(); 
        
        while (1) {
            
            k3 = 1;
            if (b.read()) {
                f();
                k3 = 2;
            }
            
            if (c.read()) {
                g();
                k3 = 4;
            }

            wait();  // 3   
        }
    }
    

    // break in while loop in function
    void f1() 
    {
        while (a.read()) {
            wait();
            
            if (b.read()) break;
        }
    }
    
    void break_via_func4() 
    {
        wait(); 
        
        while (1) {
            f1();
            sct_assert_level(1);
            wait();
        }
    }

    // break in function with return
    sc_signal<int> si;
    int f2() 
    {
        int res = 1;
        while (a.read() || b.read()) {
            res = si.read();
            wait();
            if (res == 1) break;
        }
        return res;
    }
    
    void break_via_func5() 
    {
        wait(); 
        
        while (1) {
            int i = f2();
            f1();
            sct_assert_level(1);
            wait();
        }
    }    
// --------------------------------------------------------------------------
    
    int f3(int i) {
        return (i+1);
    }
    
    // Break from loop followed by function call
    void fcall_break1() 
    {
        k4 = 0;
        wait();
        
        while (true) 
        {
            for (int i = 0; i < 2; i++) {
                if (a.read()) {
                    k4 = 1;
                    break;
                }
                wait(); // 2
            }

            k4 = f3(k4);
            wait();     // 3
        }
    }  
    
      
};

class B_top : public sc_module 
{
public:

    sc_signal<bool>  clk{"clk"};
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};
    sc_signal<bool>  c{"c"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
    }
};

int  sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
