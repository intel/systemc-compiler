/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// CTHREAD reset local variables initialization at various level, 
// function parameters and return, array, record and record arrays
class A : public sc_module {
public:
    sc_in<bool> clk;
    sc_signal<bool> rst;
    sc_signal<int> s;

    SC_CTOR(A) 
    {
        SC_CTHREAD(thread1, clk.pos()); 
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(thread_array1, clk.pos()); 
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(thread_record1, clk.pos()); 
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(thread_fcall1, clk.pos()); 
        async_reset_signal_is(rst, 0);
    }

    sc_signal<int> r1;
    void thread1() {
        int n;          
        int m;
        m = 41;
        r1 = m + n;

        int i;
        int k = 42;     // Normal init
        if (k > m) {
            i = 42;
            int j = i + k + n + 1;
            r1 = j;
        }
        wait();
        
        while(true) {
            int l;
            l = k;
            r1 = l;
            wait();
        }
    }
    
    sc_signal<int> r2;
    void thread_array1() {
        int am[3];              // No init, register
        int at[2] = {42, 43};   // Normal init
        int aa[3];
        sc_int<16> ab[3];

        r2 = at[1] + aa[0];

        if (at[1] > ab[1]) {
            int ac[3];
            r2 = ab[1] + ac[1] + at[1] + aa[0];
        }
        wait();
        
        while(true) {
            int l[3];
            l[1] = am[0];
            r2 = l[1];
            wait();
        }
    }
    
// ---------------------------------------------------------------------------    
    struct Simple {
        int a;
        bool b;
        sc_uint<4> x;
        Simple() {}
        Simple(int par) : a(par), b(par==42), x(par+1) {}
    };
    
    sc_signal<int> r3;
    void thread_record1() {
        Simple rec;
        
        if (s.read()) {
            Simple recarr[3];
            r3 = recarr[1].x + rec.x;
            recarr[1].a = 1;
        } else {
            r3 = 0;
        }
        wait();
        
        while(true) {
            Simple rec_;
            r3 = rec.x + rec_.x;
            wait();
        }
    }
    
// ---------------------------------------------------------------------------    
    sc_signal<int> r4;
    int g(int par) {return (par+1);}
    int g(int& par1, const int& par2) {return (par1+par2);}
    int h() {
        int l = 42;
        return l;
    }
    int h(bool b) {
        int l = 42;
        if (b) {
            return l;
        } else {
            return 0;
        }
    }
    
    void thread_fcall1() {
        int i = g(1);
        i = g(i);
        i = g(i, 42);
        r4 = i;
        wait();
        
        while(true) {
            i = h();
            r4 = 0;
            wait();
        }
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

