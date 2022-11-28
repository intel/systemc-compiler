/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

// Replace constants and template parameters by values 
template<unsigned N>
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;
    sc_signal<int> t;
    
    static const int M = N+1;
    const int       C1 = 42;
    const int       C2 = C1+1;
    const unsigned  C3 = 42;
    const unsigned  C4 = 42;
    const int       D1 = 0;     
    const unsigned  D2 = 0;
    const unsigned  D3 = 0;
    
    const int       N1 = -1;
    static const int N2 = -2;
    const int NARR[3] = {-1, -2, -3};
    
    const unsigned ARR[3] = {1, 2, 3};
    const unsigned ARR1 = ARR[1];

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        {
            auto& d = const_cast<int&>(D1);
            d = 43;
        }
        {
            auto& d = const_cast<unsigned&>(D2);
            d = 43;
        }
        {
            auto& d = const_cast<unsigned&>(D3);
            d = 43;
        }
        SC_METHOD(const_range1);
        sensitive << s;
        
        SC_METHOD(const_range2);
        sensitive << s;

        SC_CTHREAD(multi_ref_call, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(negative_cost_ref1, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(negative_cost_ref2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(negative_cost_ref3, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(static_const_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(sig_init_method);
        sensitive << s << t;
        
        SC_CTHREAD(sig_init_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(no_sens_method);

        SC_METHOD(const_method);
        sensitive << s;
        
        SC_METHOD(const_array_method);
        sensitive << s;

        SC_CTHREAD(const_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(const_local_array_method);
        sensitive << s;

        SC_CTHREAD(const_local_array_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(const_ref_call_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(const_record_method);
        sensitive << s;

        SC_CTHREAD(const_record_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(const_loc_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
   
//-----------------------------------------------------------------------------
    // Constant variable for range/bit
    
    const sc_uint<3> L = 1;
    const sc_uint<4> marr[3] = {4,5,6};
    
    const sc_uint<3> S = 6;
    
    void const_range1()  
    {
        const sc_uint<12> R = 7;                 // Declared
        const sc_uint<12> RR = 7;                // Declared
        const sc_uint<12> T = 8;                 // Not declared
        const sc_uint<4> larr[3] = {1,2,3};
        int l;
        
        l = S.bit(1); 
        l = R.bit(1);  
        l = RR.range(2,1);  
        l = T;  
        
        l = L.range(2,1);
        l = larr[2].range(2,1);
        l = marr[0].bit(1);
        
        l = larr[s.read()].range(2,1) + marr[s.read()].bit(1);
    }
    
    void cval(const sc_uint<3> val) {
        int l = val.range(2,1);
    }
    
    void const_range2() {
        cval(1);
    }
    
    
//-----------------------------------------------------------------------------
    // negative number in const ref
    
    sc_signal<sc_int<4>> s1;
    void cref1(const sc_int<4>& par) {
        s1 = par;
    }

    sc_signal<long int> s2;
    void cref2(const long int& par) {
        s2.write(par+1);
    }

    sc_signal<sc_int<4>> s3;
    template<class T>
    void cref3(const T& par) {
        sc_int<4> l = 1;
        wait();
        l += par;
        s3 = l;
    }

    // Check no parameter declared if referenced variable is used, for #246
    const sc_int<4> XC = 3;
    void multi_ref_call() 
    {
        wait();
        while (true) {
            const sc_int<4> y = 41;
            cref1(y);
            cref1(XC);
            wait();
        }
    }
    
    void negative_cost_ref1() 
    {
        wait();
        while (true) {
            sc_int<4> x = -3;
            cref1(x+1);
            wait();
            cref1(N1);
        }
    }
    
    void negative_cost_ref2() 
    {
        wait();
        while (true) {
            cref2(-42);
            cref2(-43);
            wait();
            cref2(NARR[s.read()]);
            wait();
        }
    }

    void negative_cost_ref3() 
    {
        wait();
        while (true) {
            const sc_int<4> y = ~(sc_int<4>)5;
            sct_assert_const(y == -6);
            cref3(y);
            cref3(NARR[2]);
        }
    }
    
    
    //-------------------------------------------------------------------------
    
    int ff() {
        return 42;
    }
     
    void static_const_thread() 
    {
        const int c = ff();
        const int cs = ff()-1;
        static const int sc = 51;
        wait();
        
        while (1) {
            static const sc_uint<12> scu = 52;
            int k = c % cs;
            k = sc + scu;
            wait();
        }
    }
    
    //-------------------------------------------------------------------------

    // Check constant removed for assertion
    static const unsigned MUX_RATE = 4;
    SCT_ASSERT(s.read() != MUX_RATE-1, clk.pos());
    
    
    //-------------------------------------------------------------------------
    // Constant initialized from signal
    void sig_init_method() {
        const int LC0 = 42;
        const int LC1 = t.read();
        const sc_uint<4> LC2 = s.read();
        int h = LC0 + LC1 + LC2;
    }

    int f() {return 42;}
    const int TC0 = f();
    
    void sig_init_thread() {
        const int TC1 = t.read();   
        const int TC2 = f();        
        const int TC3 = 43;
        wait();
        
        while (true) {
            const sc_uint<4> TC4 = s.read();
            const int TC5 = f();
            const int TC6 = 44;
            int n = TC4 + TC5 + TC6;
            int m = TC0 + TC1 + TC2 + TC3;
            wait();
        }
    }   
    
    //-------------------------------------------------------------------------

    void no_sens_method() {
        const int L11 = 45;
        int a = C2 + D1 + L11;
    }

    void const_method() {
        const int L10 = 45;
        unsigned b = C3 + D2 + L10 + M;
    }
  
    void const_array_method() {
        
        unsigned sum = 0;
        for (int i = 0; i < 3; ++i) {
            sum += ARR[i];
        }
        int e = ARR1;
    }

    void const_thread() {
        const int L1 = 44;
        const int L2 = 44;
        const int L3 = 44;
        int c = L2;
        wait();
        
        while (true) {
            const int L4 = 44 + N;
            unsigned d = C4 + L1 + L4 + D3;
            wait();
            const int L5 = 44;
        }
    }

    void const_local_array_method() {
        
        const unsigned LARR[3] = {1, 2, 3};
        const unsigned LLARR[3] = {1, 2, 3};  // Could be removed, but not
        const unsigned LARR1 = LARR[1];
        
        unsigned lsum = 0;
        for (int i = 0; i < 3; ++i) {
            lsum += LARR[i];
        }
        int f = LARR1 + LLARR[1];
    }

    void const_local_array_thread() {

        const unsigned TARR[3] = {1, 2, 3};
        const unsigned TARR1 = TARR[1];
        wait();
        
        while (true) {
            unsigned tsum = 0;
            for (int i = 0; i < 3; ++i) {
                tsum += TARR[i];
            }
            int g = TARR1;
            
            wait();
        }
    }    
    
    //-------------------------------------------------------------------------

    void g(const int& par) {
        int aa = par + 1;
    }
    
    void const_ref_call_thread() {
        wait();
        
        while (true) {
            g(C1);
            
            wait();
        }
    }    
    
    //-------------------------------------------------------------------------
    // Constant record
    
    struct Simple {
        bool a;
        sc_uint<4> b;
        
        Simple(bool a, sc_uint<4> b) : a(a), b(b) 
        {}
    };
    
    const Simple grec1{false, 4};
    const Simple grec2{true, 5};
    
    void const_record_method() 
    {
        const Simple rec(false, 1);
        bool c = rec.a || grec1.a;
    }

    void const_record_thread() 
    {
        const Simple trec1{false, 1};       // declared at module scope
        bool c = trec1.a;
        
        wait();
        
        while (true) {
            const Simple trec2{true, 2};    
            const Simple trec3{true, 3};    
            int i = trec1.b + trec2.b;
            wait();
            i = trec3.b + grec2.b;
        }
    }    
    
    void const_loc_thread() 
    {
        const int EE = 22;
        const int DD = EE+1;
        wait();
        
        while (true) {
            const int CC = 11;
            wait();
            int i = CC + DD;
        }
    }    
};

int sc_main(int argc, char *argv[]) 
{
    A<2> a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

