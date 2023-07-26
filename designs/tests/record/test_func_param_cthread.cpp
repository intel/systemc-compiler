/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Record parameters of function passed by value and by reference
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;

    SC_CTOR(A) 
    {
        SC_CTHREAD(record_fcall_ref, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_val_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_val_comb, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(record_fcall_two_val, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_two_val2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_two_ref, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_two_ref2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_const_ref1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_const_ref2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_fcall_const_ref3, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    struct Simple {
        bool a;
        int b;
    };
    
    void f1(Simple& par) {
        bool b = par.a;
        par.b = 2;
    }
    
    void f1_const(const Simple& par) {
        bool b = !par.a;
    }
    
    sc_signal<int> t1;
    void f2(Simple par) {
        bool b = par.a;
        par.b = 2;
        t1 = b;
    }
    
    sc_signal<int> t2;
    void f3(Simple par1, Simple par2) {
        bool b = par1.a || par2.a;
        par1.a = b+1;
        par2.a = b-1;
        t2 = par2.a;
    }
    
    void f4(Simple& par1, const Simple& par2) {
        bool b = par1.a && par2.a;
        par1.a = b;
    }

    // Parameter by reference and constant reference
    sc_signal<int> t0;
    void record_fcall_ref() 
    {
        Simple s;
        wait();
        
        while (true) {
            s.b = 1;
            f1(s);
            wait();
            
            f1_const(s);
            t0 = s.b;
        }
    }

    // Parameter by value -- register
    void record_fcall_val_reg() 
    {
        Simple s;
        s.b = 1;
        wait();
        
        while (true) {
            f2(s);
            wait();
        }
    }

    // Parameter by value -- comb
    void record_fcall_val_comb() 
    {
        wait();
        while (true) {
            Simple s;
            s.b = 1;
            f2(s);
            wait();
        }
    }

    // Two record parameters by value
    void record_fcall_two_val() 
    {
        Simple s; 
        wait();
        while (true) {
            Simple r;
            f3(s, r);
            wait();
        }
    }
    
    // Global and local record parameters by value
    Simple gr;
    void record_fcall_two_val2() 
    {
        wait();
        while (true) {
            Simple s;
            gr.a = true;
            f3(gr, s);
            wait();
        }
    }
    
    // Two record parameters by reference
    sc_signal<int> t3;
    void record_fcall_two_ref() 
    {
        wait();
        while (true) {
            Simple s; 
            wait();
            Simple r;
            f4(s, r);
            t3 = s.a + r.a;
        }
    }
    
    // Global and local record parameters by reference
    Simple gs;
    sc_signal<int> t4;
    void record_fcall_two_ref2() 
    {
        Simple r;
        wait();
        while (true) {
            r.b = 4;
            f4(gs, r);
            t4 = r.b;
            wait();
        }
    }
    
// ---------------------------------------------------------------------------    
    // Constant reference parameters
    
    void cref_copy(Simple& par1, const Simple& par2) {
        par1 = par2;        
    }
    
    sc_signal<int> t5;
    void record_fcall_const_ref1() 
    {
        Simple r; Simple t;
        wait();
        while (true) {
            cref_copy(r, t);
            r = t;
            wait();
            bool b = cref_cmp(r,t);
            t5 = b;
        }
    }
    

    bool cref_cmp(const Simple& par1, const Simple& par2) {
        return (par1.a == par2.a && par1.b == par2.b);
    }

    sc_signal<int> t6;
    void record_fcall_const_ref2() 
    {
        Simple r; Simple t;
        wait();
        while (true) {
            t.a = 1;
            wait();
            bool b = cref_cmp(r,t);
            t6 = b;
        }
    }
    
    int cref_sum(const Simple& par) {
        wait();
        int res = par.a + par.b;
        return res;
    }

    sc_signal<int> t7;
    void record_fcall_const_ref3() 
    {
        Simple r; 
        wait();
        while (true) {
            Simple t;
            int i = cref_sum(t);
            i = cref_sum(r);
            t7 = i;
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);

    sc_start();
    return 0;
}

