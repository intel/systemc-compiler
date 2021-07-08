/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local variables init at various level, including function parameters and return, 
// array, record and record arrays
class A : public sc_module {
public:
    sc_in<bool> clk;
    sc_signal<bool> rst;
    sc_signal<int> s;

    SC_CTOR(A) 
    {
        SC_METHOD(method1); sensitive << s;
        
        SC_CTHREAD(thread1, clk.pos()); 
        async_reset_signal_is(rst, 0);
        
        SC_METHOD(method_array1); sensitive << s;
        
        SC_CTHREAD(thread_array1, clk.pos()); 
        async_reset_signal_is(rst, 0);

        SC_METHOD(method_record1); sensitive << s;
        SC_METHOD(method_record_array1); sensitive << s;
        
        SC_METHOD(fcall_param1); sensitive << s;
        SC_METHOD(fcall_return1); sensitive << s;
        SC_METHOD(fcall_return2); sensitive << s;
        SC_METHOD(ptr_and_ref); sensitive << s;
        
        
        SC_CTHREAD(thread_for1, clk.pos()); 
        async_reset_signal_is(rst, 0);
    }

    sc_signal<int> r1;
    void method1() {
	int i;
        sc_uint<4> x;           // Normal init as SC type
        int k = 42;             // Normal init
        r1 = 0;
        if (s.read()) {
            i = 42;
            sc_uint<4> y;
            int j = i + k + y + x + 1;
            r1 = j;
        }
    }
    

    sc_signal<int> r2;
    void thread1() {
        int n;          // No init
        int rl;         // No init
        rl = 41;
        r2 = rl;
        wait();
        
        while(true) {
            int i;
            int k = 42;         // Normal init
            if (s.read()) {
                i = 42;
                int j = i + k + n + 1;
                r2 = j;
            }
            wait();
        }
    }
    
    sc_signal<int> r4;
    void method_array1() 
    {
        bool an[3] = {};        // No init    
        int ar[3] = {1,2,3};    // No init    
	int a[3];
        sc_int<8> ax[3];        // No init    
        
        if (s.read()) {
            sc_bigint<65> ay[3];
            int b[4] = {an[0], ar[1]};
            
            r4 = a[0] + b[1] + ax[2] + ay[2].to_int();
        } else {
            r4 = 0;
        }
    }
    
    sc_signal<int> r5;
    void thread_array1() {
        int am[3];              // No init, register
        int at[2] = {42, 43};   // No init
        r5 = at[1];
        wait();
        
        while(true) {
            int aa[3];
            int ab[3] = {4,5,6}; // Normal init
            
            if (s.read()) {
                int ac[3] = {7,8,9};
                int j = aa[1] + ab[1] + ac[1] + am[1];
                r5 = j;
            }
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
    sc_signal<int> r6;
    void method_record1() 
    {
        Simple reca;

        if (s.read()) {
            Simple recb(1);
            r6 = reca.x + recb.a;
        } else {
            Simple recc[3];
            r6 = recc[1].a;
        }
    }
    
    sc_signal<int> r7;
    void method_record_array1() 
    {
        Simple recarr1[2] = {};
        
        if (s.read()) {
            Simple recarr2[3];
            r7 = recarr1[1].x;
        } else {
            r7 = 0;
        }
    }
    
// ---------------------------------------------------------------------------    
    sc_signal<int> r8;
    void f(int par) {r8 = par;}
    void fcall_param1() 
    {
        f(1);
        int i = 42;
        f(i);
        r8 = i;
    }
    
    sc_signal<int> r9;
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
    void fcall_return1() 
    {
        if (s.read()) {
            int i = g(1);
            i = g(i);
            i = g(i, 42);
            i = h();
            r9 = i;
        }
        r9 = 0;
    }
    
    sc_signal<int> r10;
    void fcall_return2() 
    {
        int i = g(1);
        i = g(i);
        i = g(i, 42);
        i = h();
        int j = h(true);    // See #263
        r10 = j;
    }
    
    int m;
    int* p = &m;
    int* q = nullptr;
    void ptr_and_ref() 
    {
        int* lp = p;
        int& lr = m;
        int i = 42;
        int& lr1 = i;
        lr = 43 + lr1 + *p;
    }

// ---------------------------------------------------------------------------    
    
    // No zero init required for loop counter
    sc_signal<int> r3;
    void thread_for1() {
        int a[4];
        for (int i = 0; i < 4; ++i) {
            a[i] = 0;
        }    
        wait();
        
        while(true) {
            if (s.read()) {
                for (int i = 0; i < 4; ++i) {
                    a[i] = 1;
                } 
                for (int i = 0; i < 4; ++i) {
                    a[i] = 2;
                    wait();
                } 
            }
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

