/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

// do/while with complex condition with &&/|| constant LHS/RHS
class A : public sc_module 
{
public:
    sc_in<bool>         clk;
    sc_signal<bool>     rstn;
    
    sc_signal<bool>     s;

    SC_CTOR(A) 
    {
        SC_CTHREAD(const1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const3, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(const4, clk.pos());
        async_reset_signal_is(rstn, false);


        SC_CTHREAD(const_wait1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait2a, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait2b, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(const_wait3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait4, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait5, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait6, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_wait7, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // -----------------------------------------------------------------------
    // Non-wait while with complex condition
    
    sc_signal<bool> s1;
    void const1()
    { 
        const bool A = 0;
        const bool B = 1;
        s1 = 0;
        wait();

        while (true) 
        {
            do {
                s1 = 1;
            } while (B && s.read());
            wait();
        }
    }
    
    sc_signal<bool> s2;
    void const2()
    { 
        const bool A = 0;
        const bool B = 1;
        s2 = 0;
        wait();                         

        while (true)                    
        {
            do {     
                s2 = 1;                 
            } while (s.read() || A);
            wait();                     
        }
    }
    
    sc_signal<bool> s3;
    void const3()
    { 
        const bool A = 0;
        const bool B = 1;
        s3 = 0;
        wait();

        while (true) 
        {
            do {
                s3 = 1;
            } while (A || B && s.read() == 1 && !B);
            wait();
        }
    }
    
    sc_signal<bool> s4;
    void const4()
    { 
        const bool A = 0;
        const bool B = 1;
        s4 = 0;
        wait();

        while (true) 
        {
            do {
                s4 = 1;
            } while (A || s.read() && (s2.read() && A) || s3.read());
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
            do {     
                s5 = !s5;
                wait();
            } while (s.read() || A);
            wait();                     
        }
    }
    
    
    // empty body
    sc_signal<int> s6;
    void const_wait2()
    { 
        const bool A = 0;
        s6 = 0;
        wait();

        while (true)            // B5
        {
            do {} while (A);    // B3
            
            s6 = 1;             // B2
            wait();
        }
    }
    
    sc_signal<int> s6a;
    void const_wait2a()
    { 
        s6a = 0;
        wait();

        while (true)            // B5
        {
            do {} while (s.read()==1);    // B3
            
            s6a = 1;             // B2
            wait();
        }
    }
    
    sc_signal<int> s6b;
    void const_wait2b()
    { 
        s6b = 0;
        wait();

        while (true)            // B5
        {
            do {
                wait();
                
                s6b = 1;

                do {} while (s.read()==1);    // B3
                
            } while (s.read());
            
            s6b = 2;
            wait();
        }
    }
    
    sc_signal<int> s7;
    void const_wait3()
    { 
        const bool A = 0;
        s7 = 0;
        wait();

        while (true) 
        {
            do {                // B6, B5
                s7 = 2;     
            } while (A);        // B3
            
            s7 = 3;             // B2
            wait();                     
        }
    }
  
    // false condition do/while with wait()
    sc_signal<int> s8;
    void const_wait4()
    { 
        const bool A = 0;
        s8 = 0;
        wait();

        while (true) 
        {
            do {                // B6, B5
                s8 = 2;         // B4
                wait();     
            } while (A);        // B3
            
            s8 = 3;             // B2
            wait();                     
        }
    }
    
    
    sc_signal<int> s9;
    void const_wait5()
    { 
        const bool A = 0;
        s9 = 0;
        wait();

        while (true) 
        {
            do {
                s9 = 1;
                wait();                 // 1
                
                if (s.read()) break;
                s9 = 2;
                
            } while (s.read() && A);
            s9 = 3;
            wait();                     // 2
        }
    }     
    
    sc_signal<int> s10;
    void const_wait6()
    { 
        const bool A = 0;
        const bool B = 1;
        s10 = 0;
        wait();

        while (true) 
        {
            do {
                if (s1.read()) {
                    wait();             // 1
                    s10 = 1;
                    continue;
                }
                wait();                 // 2
                s10 = 2;
                
                do {
                    wait();             // 3
                    s10 = 3;
                } while (!s2.read() || A && B);
            } while (s3.read() && B);
            if (s4.read()) s10 = 4;
            wait();                     // 4
        }
    }
    
    sc_signal<int> s11;
    void const_wait7()
    { 
        s11 = 0;
        wait();

        while (true) 
        {
            do {
                if (s1.read()) {
                    wait();             // 1            
                    s11 = 1;
                    continue;
                }
                wait();                 // 2   
            } while (s2.read() && 0);
            wait();                     // 3

            do {
                wait();                 // 4
                s11 = 2;
            } while (0 && (1 || s3.read()));
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
