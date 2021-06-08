/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Defined and read for variable initialization and unary/binary operations
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    sc_signal<sc_uint<8>> s;
    
    SC_CTOR(A) {
        
        SC_METHOD(read_array_bug); sensitive << s;

        SC_CTHREAD(linear1, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_METHOD(linear_self); sensitive << s;
        SC_CTHREAD(linear_comp_assign, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_CTHREAD(linear_unary, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(decl_init, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(decl_init_array, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_METHOD(read_array); sensitive << s;
        SC_METHOD(wite_known_array); sensitive << s;
        SC_METHOD(wite_unknown_array); sensitive << s;

        SC_METHOD(linear_scint_oper); sensitive << s;
        SC_METHOD(linear_scint_member); sensitive << s;
        SC_METHOD(linear_scfunc_call); sensitive << s;
        
        SC_METHOD(sc_type_init); sensitive << s;
        SC_METHOD(sc_type_init2); sensitive << s;
        SC_METHOD(sc_type_binary); sensitive << s;
        SC_METHOD(sc_type_unary); sensitive << s;

        SC_METHOD(if_stmt); sensitive << s;
        SC_METHOD(cond_oper); sensitive << s;
    }
    
    // Fixed bug of unknown array element assigned
    void read_array_bug()
    {
        int a[3];
        int b[2][2];
        sc_uint<4> c[3];
        
        int i;
        i = a[s.read()];
        i = c[i] + 1;
        i = b[1][s.read()];
        i = c[i] -1;
    }
    

    int                 m;
    int                 k;
    int                 n;
    const int           CI = 1;
    void linear1()
    {
        m = 1;
        int i = 1;
        k = m + CI;
        m = k - 1;
        n = k;
        sct_assert_defined(i);
        sct_assert_defined(k);
        sct_assert_defined(n);
        sct_assert_defined(m);
        sct_assert_read(k);
        sct_assert_read(n, false);
        wait();
        
        while(1) wait();
    }

    // Assignment with the same variable in left and right parts
    int                 m1;
    int                 k1;
    void linear_self()
    {
        k1 = k1 + 1;
        m1 = ~m1;
        sct_assert_register(k1);
        sct_assert_register(m1);
    }
    
    int                 m2;
    int                 k2;

    void linear_comp_assign()
    {
        k2 = 0; m2 = 0;
        int i = 1;
        i += 1;
        k2 -= m2;

        sct_assert_defined(i);
        sct_assert_read(i);
        sct_assert_defined(k2);
        sct_assert_read(k2);
        sct_assert_read(m2);
        wait();
        
        while(1) wait();
    }

    int                 n3;
    int                 m3;
    int                 k3;

    void linear_unary()
    {
        k3 = 0; m3 = 1; n3 = 2;
        int i = +k3;
        i = --m3;
        n3++;

        sct_assert_read(k3);
        sct_assert_defined(m3);
        sct_assert_read(m3);
        sct_assert_defined(n3);
        sct_assert_read(n3);
        wait();
        
        while(1) wait();
    }
    
    void decl_init()
    {
        int i = 0;
        int ii;
        int j = i;
        int a[3];
        int b[2] = {1,2};
        bool c[] = {true,false,true};
        sc_uint<2> d[2];
        sc_uint<3> e[] = {1,2,j};
        
        sct_assert_defined(i);
        sct_assert_read(i);
        sct_assert_read(ii, false);
        sct_assert_defined(a, false);
        sct_assert_defined(b);
        sct_assert_defined(c);
        sct_assert_defined(d);
        sct_assert_defined(e);
        sct_assert_read(j);
        wait();
        
        while(1) wait();
    }

    void decl_init_array()
    {
        int a[3] = {1, 2, 3};
        int b[2][2] = {1, 2, 3, 4};
        sc_uint<4> c[2][3];
        int d[4];
        
        for(int i = 0; i < 4; ++i) {
            d[i] = i;
        }
        
        sct_assert_defined(a);
        sct_assert_defined(b);
        sct_assert_defined(c);
        sct_assert_array_defined(d);
        wait();
        
        while(1) wait();
    }
    

    void read_array()
    {
        int a[3] = {1, 2, 3};
        int b[2][2];
        sc_uint<4> c[3];
        bool d[4];
        sc_int<1> e[4];
        sc_biguint<33> f[10];
        
        int i = a[1];
        i = b[1][s.read()];
        i = c[i] + 1;
        
        i = d[s.read()-1] ? 1 : 2;
        
        sct_assert_read(a);
        sct_assert_read(b);
        sct_assert_read(c);
        sct_assert_read(d);
        sct_assert_read(e, false);
        
        e[s.read()]++;

        for(int i = 0; i < 4; ++i) {
            d[i] = 1 << f[i+1].to_int();
        }

        sct_assert_read(e);
        sct_assert_read(f);
    }
    
    // Write to array element by known index
    void wite_known_array()
    {
        int a[3];
        bool b[2][2];
        unsigned c[2][3];
        long d[2][2];

        const int l = 0;
        sc_uint<4> x = 2;
        a[1] = 1;
        a[x] = 2;
        b[l][1] = true;
        c[l+1][2] = 3;
        
        sct_assert_array_defined(a);
        sct_assert_array_defined(b);
        sct_assert_array_defined(c);
    }
     
    // Write to array element by unknown index
    void wite_unknown_array()
    {
        int a[3];
        int b[2][2];
        int c[2][3];
        
        int i = s.read();
        a[i]= 1;
        a[s.read()+1] = 1;
        b[i][1] = 2;
        c[1][s.read()] = 3;
        
        int& ri = i;
        a[ri] = a[ri-1];
        
        const sc_uint<8>& r = s.read();
        a[r] = 1;
        a[r+1] = 1;
        
        sct_assert_array_defined(a);
        sct_assert_array_defined(b);
        sct_assert_array_defined(c);
    }
    
    void linear_scint_oper()
    {
        int i = 1;
        int j1 = 3;
        int j2 = 2;
        
        sc_int<8> x;
        x[i] = 1;
        x(j1, j1-1) = 1;
        x(j2+1, j2) = 1;
        
        sct_assert_read(i);
        sct_assert_read(j1);
        sct_assert_read(j2);

        sc_bigint<33> y;
        int k = i;
        int& rk = k;
        y[rk] = 1;
        y.bit(rk+1) = 0;
        y(rk+1, rk) = 0;
        
        sct_assert_defined(k);
        sct_assert_read(k);
        sct_assert_defined(y);

        const sc_uint<8>& r = s.read();
        y.bit(r) = 0;
        y(r+1, r-1) = 0;
        
        sct_assert_read(s);
    }
    
    void linear_scint_member()
    {
        int i = 1;
        int j1 = 2;
        int j2 = 3;
        sc_int<8> x;
        x.bit(i) = 1;
        x.range(j1, j1-1) = 1;
        x.range(j2+1, j2) = 1;
        
        sct_assert_read(i);
        sct_assert_read(j1);
        sct_assert_read(j2);
    }
    
    void linear_scfunc_call()
    {
        sc_int<3> x;
        sc_int<3> y;
        sc_uint<6> z = concat(x, y);
        bool b = and_reduce(z);

        sct_assert_read(x);
        sct_assert_read(y);
        sct_assert_read(z);

        b = or_reduce(s.read());
        sct_assert_read(s);
    }
    
    void sc_type_init() {
        // Check @x1 and @x2 are in @read
        sc_int<16> x1;
        sc_int<16> y(x1);
        sc_int<16> x2;
        sc_int<16> z = x2;
        
        sct_assert_defined(y);
        sct_assert_defined(z);
        sct_assert_read(x1);
        sct_assert_read(x2);
    }
   
     void sc_type_init2() {
        sc_int<4> x;
        sc_int<4> y;
        sc_int<8> z = x + y;
        
        sct_assert_defined(z);
        sct_assert_read(x);
        sct_assert_read(y);
    }

    void sc_type_binary() {
        sc_int<4> x;
        sc_int<4> y;
        sc_int<8> z;
        z = x + y;
        
        sct_assert_defined(z);
        sct_assert_read(x);
        sct_assert_read(y);
        
        sc_int<4> x2;
        sc_int<4>& rx = x2;
        sc_int<8> z2;
        
        z2 = s.read() + rx;
        sct_assert_defined(z2);
        sct_assert_read(s);
        sct_assert_read(x2);
    }
    
    void sc_type_unary() {
        sc_int<4> x;
        sc_int<4> y;
        bool b;
        sc_int<4> z;
        b = !x;
        z = ~y;
        
        sct_assert_defined(b);
        sct_assert_defined(z);
        sct_assert_read(x);
        sct_assert_read(y);
    }    
    
    void if_stmt()
    {
        bool c1 = s.read();
        int i1, j1;
        if (c1) {
            int k = i1;
        } else {
            int k = j1;
        }
        sct_assert_read(c1);
        sct_assert_read(i1);
        sct_assert_read(j1);
    }
    
    void cond_oper()
    {
        bool c1 = s.read();
        int i1, j1;
        int k1 = c1 ? i1 : j1;
        sct_assert_read(c1);
        sct_assert_read(i1);
        sct_assert_read(j1);

        int i2, j2, c2;
        int k2 = (c2 == 0) ? i2+1 : j2+1;
        sct_assert_read(c2);
        sct_assert_read(i2);
        sct_assert_read(j2);
    }    
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk("clk", 1, SC_NS);
    
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

