/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include "sct_assert.h"
#include "systemc.h"

using namespace sct;

// Check unused variables/statements remove for local and member array
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
        SC_METHOD(remove_local); sensitive << s << r;
        SC_METHOD(remove_member); sensitive << s << r;
        SC_METHOD(remove_ref); sensitive << s << r;
        SC_METHOD(not_remove_ref); sensitive << s << r;

        SC_CTHREAD(remove_local_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_ref_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> t0b;
    void remove_local() 
    {
        int i1[3] = {1,2,3};            // removed

        unsigned u1[3] = {1,2,3};       // not removed
        t0b = u1[0];

        unsigned u2[3] = {1,2,3};       // not removed
        t0b = u2[s.read()];
        
        unsigned u3[3] = {1,2,3};       // not removed
        if (s.read()) t0b = u3[s.read()+1];
        
        int i2[3] = {1,2,3};            // not removed
        int i3[3];                      // not removed
        i3[s.read()] = s.read();        // not removed
        t0b = i2[i3[r.read()]];          // not removed

        const int i4[3] = {1,2,3};      // not removed
        t0b = i4[2];
        
        sc_uint<4> x1[3][4];            // removed
        for (unsigned i = 0; i < 3; ++i) 
        for (unsigned j = 0; j < 4; ++j) x1[i][j] = i*j;
        
        sc_uint<4> x2[3][4];            // not removed
        sc_uint<4> x3[2][3][4];         // not removed    
        for (unsigned i = 0; i < 3; ++i) 
        for (unsigned j = 0; j < 4; ++j) x2[i][j] = i*j;
        x3[s.read()][0][r.read()] = x2[1][s.read()];
        t0b = x3[1][1][r.read()];
    }
    
    sc_signal<int> t0a;
    void remove_local_thread() 
    {
        int i1[3] = {1,2,3};            // removed

        unsigned u1[3] = {1,2,3};       // not removed
        t0a = u1[0];

        unsigned u2[3] = {1,2,3};       // not removed
        wait();

        while (true) {
            t0a = u2[s.read()];

            unsigned u2[3] = {1,2,3};       // not removed
            t0a = u2[s.read()];

            unsigned u3[3] = {1,2,3};       // not removed
            if (s.read()) t0a = u3[s.read()+1];

            int i2[3] = {1,2,3};            // not removed
            int i3[3];                      // not removed
            i3[s.read()] = s.read();        // not removed
            t0a = i2[i3[r.read()]];          // not removed

            const int i4[3] = {1,2,3};      // not removed
            t0a = i4[2];

            sc_uint<4> x1[3][4];            // removed
            for (unsigned i = 0; i < 3; ++i) 
            for (unsigned j = 0; j < 4; ++j) x1[i][j] = i*j;

            sc_uint<4> x2[3][4];            // not removed
            sc_uint<4> x3[2][3][4];         // not removed    
            for (unsigned i = 0; i < 3; ++i) 
            for (unsigned j = 0; j < 4; ++j) x2[i][j] = i*j;
            x3[s.read()][0][r.read()] = x2[1][s.read()];
            t0a = x3[1][1][r.read()];
            wait();
        }
    }
    
    int m1[3] = {1,2,3};                // removed
    const int m2[3] = {1,2,3};          // removed
    
    int m3[3] = {1,2,3};                // not removed
    int m3t[3] = {1,2,3};               // not removed
    const sc_uint<4> m4[3][4];          // not removed
    const sc_uint<4> m4t[3][4];         // not removed
    sc_bigint<65> m5[2][1];             // not removed
    sc_bigint<65> m5t[2][1];            // not removed
    
    int m6[3];                          // not removed
    int m6t[3];                         // not removed
    int m7[3] = {1,2,3};                // not removed
    int m7t[3] = {1,2,3};               // not removed
        
    sc_signal<int> t0c;
    void remove_member() 
    {
        t0c = m3[s.read()];
        t0c = m4[s.read()][1];
        t0c = m5[s.read()][0].to_int();
        
        m6[s.read()] = m7[0];
        t0c = m6[0];
    }

    sc_signal<int> t0d;
    void remove_member_thread() 
    {
       
        wait();

        while (true) {
            t0d = m3t[s.read()];
            t0d = m4t[s.read()][1];
            t0d = m5t[s.read()][0].to_int();

            m6[s.read()] = m7t[0];
            t0d = m6t[0];
            wait();
        }
    }
    
    template<unsigned N>
    void refParamZero(sct_uint<N>& par) {
        int i = 42 + par;
        sct_uint<N> l = par;
        par = 1;
    }
    
    template<unsigned N>
    void constRefParamZero(const sct_uint<N>& par) {
        int j = 42 + par;
        sct_uint<N> l = par;
    }
    
    // Array passed as reference parameter to function -- bug fixed
    template<unsigned N>
    void remove_ref_() {
        sct_uint<N> k;
        sct_uint<N>& rk = k;
        int a = 42 + rk;

        sct_uint<N> p;
        sct_uint<N>& rp = p;
        rp = 42;

        sct_uint<N> m;
        sct_uint<N>& rm = m;
        refParamZero<N>(rm);

        sct_uint<N> n;
        sct_uint<N>& rn = n;
        constRefParamZero<N>(rn);

        sct_uint<N> aarr[3];
        a = 42 + aarr[s.read()];

        sct_uint<N> barr[3];
        barr[s.read()] = 42;

        sct_uint<N> carr[3];
        refParamZero<N>(carr[s.read()]);
        
        sct_uint<N> darr[3];
        constRefParamZero<N>(darr[s.read()]);
    }

    sct_uint<1> x;
    sct_uint<1> y;
    sct_uint<1> z;
    void remove_ref() {
        remove_ref_<0>();
        remove_ref_<1>();
        
        sct_uint<1>& rx = x;
        rx = 42;

        sct_uint<1>& ry = y;
        refParamZero<1>(ry);

        sct_uint<1>& rz = z;
        constRefParamZero<1>(rz);
    }
    
    sc_signal<int> t0e;
    void not_remove_ref() {
        sct_uint<1> mm;
        sct_uint<1>& rl = mm;
        int a = 42 + rl;

        sct_uint<1> aarr[3];
        a = 42 + aarr[s.read()];

        sct_uint<1> barr[3];
        refParamZero<1>(barr[s.read()]);
        
        sct_uint<1> carr[3];
        constRefParamZero<1>(carr[s.read()]);
        t0e = aarr[0] + barr[0] + carr[0] + a;
    }
    
    
    void remove_ref_thread() {
        wait();
        while (true) {
            remove_ref_<0>();
            wait();
            remove_ref_<1>();
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

