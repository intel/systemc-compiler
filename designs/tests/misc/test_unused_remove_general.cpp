/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Check unused variables/statements remove dependable variables and statements
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;
    sc_signal<sc_uint<4>> r;

    sc_signal<int>  t0;
    sc_signal<int>  t1;
    sc_signal<int>  t2;
    sc_signal<int>  t3;
    sc_signal<int>  t4;
    sc_signal<int>  t5;
    sc_signal<int>  t6;
    sc_signal<int>  t7;
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(remove_vars1); sensitive << s << r;
        SC_METHOD(remove_vars2); sensitive << s << r;
        SC_METHOD(remove_vars3); sensitive << s << r;

        SC_CTHREAD(remove_vars1_thread, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(remove_empty_meth); sensitive << s;
        SC_CTHREAD(remove_empty_thread, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(remove_for); sensitive << s << r;
        SC_METHOD(remove_if); sensitive << s << r;
        SC_METHOD(remove_while); sensitive << s << r;

        SC_CTHREAD(remove_const_cond, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(remove_unary); sensitive << s << r;
        SC_METHOD(remove_compound); sensitive << s << r;
    }
    
    void remove_vars1() 
    {
        // not removed
        int i;
        i = s.read();
        int j = i + 1;
        int k = 1 << j;
        int l = k == r.read() ? 1 : 2;
        t0 = l;
    }
    
    void remove_vars2() 
    {
        // not removed
        sc_int<6> i;
        sc_int<6> j;
        sc_int<6> m;
        if (s.read()) {
            j = i + r.read();
        }
        for (int k = 0; k < 3; ++k) {
            if (r.read()) break;
            m = s.read() || j == k;
        }
        t1 = m;
    }
    
    void remove_vars3() 
    {
        // not removed
        sc_uint<4> i = s.read();
        sc_uint<6> j = i + 1;

        int l = 0;
        for (int k = 0; k < j; ++k) {
            l = l + 1;
        }
        t3 = l;
    }
       
    
    void remove_vars1_thread() 
    {
        // not removed
        sc_int<6> i = 1;
        sc_int<6> j = 0;
        wait();

        while (true) {
            while (!s.read()) {j = j + i; wait();}
            i = r.read();
            wait();
        }
    }
    
    // Remove process as all the statements removed
    void remove_empty_meth() {
        int i;
        int j = i+1;
    }
    
    // Remove process as all the statements removed
    void remove_empty_thread() {
        int i = 0;
        wait();
        
        while (true) {
            i = r.read();
            int j = i + 1;
            wait();
        }
    }
    
    // Nothing should be removed
    sc_signal<int> t8;
    void remove_for()
    {     
        int k0 = 3;
        for (unsigned i0 = 0; i0 < k0; ++i0) {
            t8 = 0;
        }

        int k1 = 1;
        for (unsigned i1 = k1; i1 < 3; ++i1) {
            t8 = 1;
        }

        int k2 = 1;
        for (unsigned i2 = 0; i2 < 3; i2 = i2 + k2) {
            t8 = 2;
        }
        
        unsigned i = 0;
        for (; i < 3; ++i) {}  
        t8 = i;
    }

    // Nothing should be removed
    sc_signal<int> t9;
    void remove_if()
    {
        t9 = 0;
        
        unsigned i1 = s.read();
        if (i1) {t9 = 1;}  
        
        sc_uint<12> i2 = s.read();
        t9 = i2 == 1 ? 1 : 2; 
        
        sc_uint<12> i3 = s.read();
        bool i4 = s.read() == 2;
        t9 = i3 == 1 || i4; 
        
        sc_uint<12> i5 = s.read();
        bool i6 = s.read() == 2;
        if (i5 == 1 || i6) {
            t9 = 2;
        }
    }

    // Nothing should be removed
    sc_signal<int> t10;
    void remove_while()
    {     
        int i0 = 3;
        while (i0 > 0) {i0 = i0 - 1; t10 = 1;}
        
        int i1 = 0; sc_int<4> x = 3;
        while (i1 != x) {i1 = i1 + 1; t10 = 2;}

        int i2 = 1; 
        do {i2 = i2 * 2; t10 = 3;} while (i2 < 10);
    }
    
    
    // Test with null condition statement, useVarStmts for @k0 includes null
    // Nothing should be removed
    sc_signal<int> t11;
    void remove_const_cond() {
        
        wait();
        
        while (true) {
            int k0 = 3;
            bool b1 = 1, b2 = s.read();
            for (unsigned i0 = k0; b1 || b2; ++i0) {
                t11 = 0;
            }
            
            wait();
        }
    }
    
    void remove_unary() {
        int i = 0;
        i++;
        unsigned j;
        --j; ++j;
        unsigned k;
        auto l = k--;

        sc_uint<14> x;
        x++;
        
        sc_int<10> y = 42;
        x = --y;
        
        sc_biguint<34> z;
        z--;
        
        sc_uint<12> r1;     // not removed
        r1++;
        t4 = r1;
        
        for (int i = 0; i < 4; ++i) {
        }

        for (int i = 0; i < 4; ++i) {
            t4 = i;
        }
    }
    
    void remove_compound() {
        int i = 0;
        i += 1;
        unsigned j;
        j -= i;
        unsigned k;
        auto l = k *= 2;

        int p, q;           // not removed
        p += q;
        t5 = p;

        unsigned p1, q1;    // not removed
        p1 += 1;
        q1 += p1;
        t5 = q1;
        
        sc_uint<14> x;
        x += 1;
        
        sc_int<10> y = 42;
        x -= y;
        
        sc_uint<10> z; 
        sc_uint<10> w;      // not removed
        z -= w;
        t5 = w;
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);

    sc_start();
    return 0;
}

