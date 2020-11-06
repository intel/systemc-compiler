/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/



#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// while with complex condition with &&/|| constant LHS/RHS
class A : public sc_module 
{
public:
    sc_in<bool>         clk;
    sc_signal<bool>     rstn;
    
    sc_signal<bool>     s;

    SC_CTOR(A) 
    {
        // TODO: Fix me #173
//        SC_CTHREAD(inf_loop1, clk.pos());
//        async_reset_signal_is(rstn, false);
//        
//        SC_CTHREAD(inf_loop2, clk.pos());
//        async_reset_signal_is(rstn, false);
//
//        SC_CTHREAD(inf_loop3, clk.pos());
//        async_reset_signal_is(rstn, false);
        //~TODO
        
        SC_CTHREAD(const1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const4, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const4a, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const4b, clk.pos());
        async_reset_signal_is(rstn, false);


        SC_CTHREAD(const_wait1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait4, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // -----------------------------------------------------------------------
    // Non-wait while with non-determinable iteration number, error reported

    sc_signal<bool> t1;
    void inf_loop1()
    {
        t1 = 0;
        wait();

        while (true)
        {
            while (s.read()) {
                t1 = !t1;
            }
            wait();
        }
    }

    sc_signal<bool> t2;
    void inf_loop2()
    { 
        const bool A = 1;
        t2 = 0;
        wait();

        while (true) 
        {
            while (A) {
                t2 = t1;
            }
            wait();
        }
    }
    
    sc_signal<bool> t3;
    void inf_loop3()
    { 
        const bool A = 1;
        t3 = 0;
        wait();

        while (true) 
        {
            while (s.read() || A) {
                t3 = t2;
            }
            wait();
        }
    }
    
    // -----------------------------------------------------------------------
    // Non-wait while with complex condition
    
    sc_signal<int> s1;
    void const1()
    { 
        const bool A = 0;
        s1 = 0;
        wait();

        while (true) 
        {
            while (A || s.read()) {
                s1 = 1;
            }
            wait();
        }
    }
    
    sc_signal<int> s2;
    void const2()
    { 
        const bool A = 0;
        s2 = 0;
        wait();                         

        while (true)                    
        {
            while (s.read() || A) {     
                s2 = 1;                 
            }                           
            wait();                     
        }
    }
    
    sc_signal<int> s3;
    void const3()
    { 
        const bool A = 0;
        const bool B = 1;
        s3 = 0;
        wait();

        while (true) 
        {
            while (A || B && s.read() || A) {
                s3 = 1;
            }
            wait();
        }
    }
    
    sc_signal<int> s4;
    void const4()
    { 
        const bool A = 0;
        const bool B = 1;
        s4 = 0;
        wait();

        while (true) 
        {
            while (B && s.read() && (s2.read() || A) && (!s3.read() && B)) {
                s4 = 1;
            }
            wait();
        }
    }
    
    // empty while body
    sc_signal<int> s4a;
    void const4a()
    { 
        const bool A = 0;
        s4a = 0;
        wait();

        while (true) 
        {
            while (A) {
            }
            s4a = 1;
            wait();
        }
    }
    
    sc_signal<int> s4b;
    void const4b()
    { 
        const bool A = 0;
        s4b = 0;
        wait();

        while (true) 
        {
            while (s1.read()) {
                while (s2.read() < 3) {
                }
                s4b = 1;
                wait();
            }
            s4b = 2;
            wait();
        }
    }
    
    // -----------------------------------------------------------------------
    // wait inside while with complex condition
    
    sc_signal<bool> s5;
    void const_wait1()
    { 
        const bool A = 0;
        s5 = 0;
        wait();                         

        while (true)                    
        {
            while (s.read() || A) {     
                s5 = !s5;
                wait();
            }                           
            wait();                     
        }
    }
    
    
    sc_signal<int> s6;
    void const_wait2()
    { 
        bool A = 1;
        s6 = 0;
        wait();

        while (true) 
        {
            while (A || s.read()) {
                s6 = 1;
                wait();                 // 1
                
                if (s.read()) break;
                s6 = 2;
            }
            s6 = 3;
            wait();                     // 2
        }
    }
    
    sc_signal<int> s7;
    void const_wait3()
    { 
        const bool A = 0;
        const bool B = 1;
        s7 = 0;
        wait();

        while (true) 
        {
            while (s.read() && B) {
                if (s.read()) {
                    wait();             // 1
                    s7 = 1;
                    continue;
                }
                wait();                 // 2
                s7 = 2;
                
                while (!s.read() || A && B) {
                    wait();             // 3
                    s7 = 3;
                }
            }
            if (s.read()) s7 = 4;
            wait();                     // 4
        }
    }
    
    sc_signal<int> s8;
    void const_wait4()
    { 
        s8 = 0;
        wait();

        while (true) 
        {
            while (s.read() && 0) {
                if (s.read()) {
                    wait();             
                    s8 = 1;
                    continue;
                }
                wait();                 
            }
            wait();                     // 1

            while (0 && (1 || s.read())) {
                wait();
                s8 = 2;
            }
        }
    }
    
    
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}
