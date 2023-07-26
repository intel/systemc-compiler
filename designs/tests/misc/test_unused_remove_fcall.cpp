/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

void f1(int& par) {
    par = par + 1;
}

int f2(const int& par) {
    return 2*par;
}

int f3(int par) {
    return par+1;
}

template<class T>
T f4(T par1, const T& par2) {
    T l = par2 + 1;
    par1 = par1 + 1;
    return par1;
}

// Check unused variables/statements remove for variables related to function calls
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;

    sc_signal<int>  t0;
    sc_signal<int>  t1;
    sc_signal<int>  t2;
    sc_signal<int>  t3;
    sc_signal<int>  t4;
    sc_signal<int>  t5;
    sc_signal<int>  t6;
    sc_signal<int>  t7;
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        
        SC_METHOD(remove_fcall); sensitive << s;
        SC_METHOD(remove_mcall); sensitive << s;       

        SC_CTHREAD(remove_fcall_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_mcall_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(remove_assert, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    // Function calls, not removed for now
    void remove_fcall() 
    {
        int i1 = 1;                 // not removed
        f1(i1);                     // not removed
    
        int i2; i2 = 1;             // not removed
        int res1 = f2(i2);          // removed
        int res2 = f2(42);          // removed
    
        int i2a; i2a = 1;           // not removed
        t0 = f2(i2a);               // not removed

        int i3; i3 = 1;             // not removed
        int res3 = f3(i3);          // removed

        int i3a; i3a = 1;           // not removed
        t0 = f3(i3a);               // not removed
    
        int i3b; i3b = 1;           // not removed
        int res3b = f3(i3b);        // not removed
        t0 = res3b;                 // not removed

        unsigned i4 = s.read();     // not removed
        unsigned j4 = s.read();     // removed
        unsigned res4 = f4(i4, j4); // not removed
    }
    
    void remove_fcall_thread() 
    {
        int i1 = 1;                 // not removed
        f1(i1);                     // not removed

        int i2; i2 = 1;             // not removed
        int res1 = f2(i2);          // removed
        int res2 = f2(42);          // removed
        wait();

        while (true) {

            int i2a; i2a = 1;           // not removed
            t0 = f2(i2a);               // not removed

            int i3; i3 = 1;             // not removed
            int res3 = f3(i3);          // removed

            int i3a; i3a = 1;           // not removed
            t0 = f3(i3a);               // not removed

            int i3b; i3b = 1;           // not removed
            int res3b = f3(i3b);        // not removed
            t0 = res3b;                 // not removed

            unsigned i4 = s.read();     // not removed
            unsigned j4 = s.read();     // removed
            unsigned res4 = f4(i4, j4); // not removed
            wait();
        }
    }

    
    void m1(bool& par1, unsigned& par2, sc_uint<4>& par3) {
        if (par1) {
            par2 = par2 + 1;
        }
        par3 = par3 + par2;
    }
    
    unsigned m2(const bool& par1, const unsigned& par2, const sc_uint<4>& par3) {
        unsigned res = par1 ? par2 : 42U;
        return res;
    }
    
    
    template<class T1, class T2, class T3>
    T2 m3(T1 par1, T2 par2, T3 par3) {
        T1 l = par1;
        return par2;
    }

    // Function calls, not removed for now
    void remove_mcall() 
    {
        bool b1; b1 = true;
        unsigned u1; u1 = 42;
        sc_uint<4> x1 = s.read();
        m1(b1, u1, x1);
        
        bool b1a; b1a = s.read();
        unsigned u1a; u1a = 42;
        sc_uint<4> x1a = s.read();
        m1(b1a, u1a, x1a);
        t1 = x1a;
        
        bool b2; b2 = s.read();
        unsigned u2; u2 = s.read();
        sc_uint<4> x2 = 0;              // removed
        m2(b2, u2, x2);                 // not removed
        
        bool b2a; b2a = s.read();
        unsigned u2a; u2a = s.read();
        sc_uint<4> x2a = s.read();      // removed
        t1 = m2(b2a, u2a, x2a);         // not removed

        bool b3; b3 = true;             // not removed
        unsigned u3; u3 = 42;           // not removed
        sc_uint<4> x3 = s.read();       // not removed
        m3(b3, u3, x3);                 // removed
        
        bool b3a; b3a = true;
        unsigned u3a; u3a = 42;
        sc_uint<4> x3a = s.read();
        t1 = m3(b3a, u3a, x3a);         // not removed
    }   
    
    void remove_mcall_thread() 
    {
        bool b1; b1 = true;
        unsigned u1; u1 = 42;
        sc_uint<4> x1 = s.read();
        m1(b1, u1, x1);
        
        bool b1a; b1a = s.read();
        unsigned u1a; u1a = 42;
        sc_uint<4> x1a = s.read();
        m1(b1a, u1a, x1a);
        t1 = x1a;
        wait();

        while (true) {
            bool b2; b2 = s.read();
            unsigned u2; u2 = s.read();
            sc_uint<4> x2 = 0;              // removed
            m2(b2, u2, x2);                 // not removed

            bool b2a; b2a = s.read();
            unsigned u2a; u2a = s.read();
            sc_uint<4> x2a = s.read();      // removed
            t1 = m2(b2a, u2a, x2a);         // not removed

            bool b3; b3 = true;             // not removed
            unsigned u3; u3 = 42;           // not removed
            sc_uint<4> x3 = s.read();       // not removed
            m3(b3, u3, x3);                 // removed

            bool b3a; b3a = true;
            unsigned u3a; u3a = 42;
            sc_uint<4> x3a = s.read();
            t1 = m3(b3a, u3a, x3a);         // not removed
    
            wait();
        }
    }

    // Assertions
    void remove_assert() 
    {
        int i = 1;
        int j = s.read();
        int k = s.read()+1;
        
        SCT_ASSERT_THREAD(i && !j, SCT_TIME(1), s.read(), clk.pos());
        wait();
        
        SCT_ASSERT_THREAD(k == 42, (1,2), t1, clk.pos());
        
        while (1) {
            int l = s.read();
            sct_assert(l == s.read());
            
            wait();

        }
    }
    
    const int c1 = 42;
    const int c2 = 43;
    SCT_ASSERT(t1 == c1, (0), t2, clk.pos());
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);

    sc_start();
    return 0;
}

