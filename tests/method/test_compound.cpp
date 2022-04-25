/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_sel_type.h"
#include "sct_assert.h"
#include <iostream>

// Compound operations including with boolean arguments
class A : public sc_module 
{
public:
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};
    
    sc_in<bool>         in;
    sc_out<bool>        out;
    
    int                 m = 11;
    int                 k = 12;

    sc_signal<int> s;
    
    SC_CTOR(A) 
    {
        SC_METHOD(compound_array); sensitive << s;
        SC_METHOD(compound_array_2d); sensitive << s;

        SC_METHOD(compound_assign); sensitive << s;
        SC_METHOD(compound_assign_brackets); sensitive << s;
        SC_METHOD(compound_bool_bitwise); sensitive << s << bv[0];
        SC_METHOD(compound_bool_arithm); sensitive << s << bv[0];
        
        SC_METHOD(compound_ref_compound); sensitive << s;
        SC_METHOD(sc_compound_assign); sensitive << s;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    // Compound for array known and unknown elements
    void compound_array()
    {
        int a[3] = {1,2,3};
        sc_uint<4> x[3] = {4,5,6};
        int i = s.read();
        
        a[0] += 1;
        sct_assert_const(a[0] == 2);    
        sct_assert_const(a[1] == 2);
        sct_assert_const(a[2] == 3);
        
        a[i] += 1;
        sct_assert_unknown(a[0]);
        sct_assert_unknown(a[1]);
        sct_assert_unknown(a[2]);
        
        x[1] -= 1;
        sct_assert_const(x[0] == 4);
        sct_assert_const(x[1] == 4);
        sct_assert_const(x[2] == 6);
        
        x[i] *= 2;
        sct_assert_unknown(x[0]);
        sct_assert_unknown(x[1]);
        sct_assert_unknown(x[2]);
    }
    
    
    void compound_array_2d()
    {
        int a[2][3]; 
        sc_uint<4> x[2][3];
        for(int i = 0; i < 2; ++i) 
        for(int j = 0; j < 3; ++j) {
            a[i][j] = 3*i + j + 1;
            x[i][j] = 3*i + j + 1;
        }
            
        
        int i = s.read();
        
        a[1][1] += 1;
        sct_assert_const(a[0][0] == 1);
        sct_assert_const(a[0][1] == 2);
        sct_assert_const(a[0][2] == 3);
        sct_assert_const(a[1][0] == 4);
        sct_assert_const(a[1][1] == 6);
        sct_assert_const(a[1][2] == 6);
        
        a[i][1] += 1;
        sct_assert_unknown(a[0][0]);
        sct_assert_unknown(a[0][1]);
        sct_assert_unknown(a[0][2]);
        sct_assert_unknown(a[1][0]);
        sct_assert_unknown(a[1][1]);
        sct_assert_unknown(a[1][2]);
        
        x[0][2] -= 2;
        sct_assert_const(x[0][0] == 1);
        sct_assert_const(x[0][1] == 2);
        sct_assert_const(x[0][2] == 1);
        sct_assert_const(x[1][0] == 4);
        sct_assert_const(x[1][1] == 5);
        sct_assert_const(x[1][2] == 6);
        
        x[0][i] /= 2;
        sct_assert_unknown(x[0][0]);
        sct_assert_unknown(x[0][1]);
        sct_assert_unknown(x[0][2]);
        sct_assert_const(x[1][0] == 4);
        sct_assert_const(x[1][1] == 5);
        sct_assert_const(x[1][2] == 6);

        x[1][i] %= 2;
        sct_assert_unknown(x[1][0]);
        sct_assert_unknown(x[1][1]);
        sct_assert_unknown(x[1][2]);
    }
    

    // Compound assignments
    void compound_assign() {
        int k = 2;
        int i = 1;
        i += 1;
        m = 1;
        CHECK(i==2);
        i -= m;
        CHECK(i==1);
        i *= k;
        i /= i;
        i %= (k+m);
        i |= 2;
        i &= 3;
        i ^= 4+i;
    }
    
    // Test shows brackets required
    void compound_assign_brackets() 
    {
        int i, e, d;
        e = 2;
        d = -3;
        i = 1;
        
        i *= e + d;
        CHECK(i == -1);
        i = 12;
        i /= e - d*2 + i - 10;
        CHECK(i == 1);
        i = 5;
        i %= 1 + 1;
        CHECK(i == 1);
    }

    sc_vector<sc_signal<bool>> bv{"bv", 3};
    void compound_bool_bitwise() 
    {
        bool b = bool(s.read());
        bool bb = bool(s.read());
        sc_uint<1> a = s.read();
        sc_uint<12> x = s.read();
        sc_int<12> y = s.read();
        int i = s.read();
        
        b = b && bb;
        b = b && a;
        b = b && x;
        b = b && y;
        b = b && i;
        b = b && s.read();
        b = b && bv[1];
        
        b &= bb;        // Warning reported
        b &= a;    
        b &= x;    
        b &= y;         // Warning reported
        b &= i;         // Warning reported
        b &= s.read();  // Warning reported
        b &= bv[1];     // Warning reported

        b |= bb;        // Warning reported
        b |= s.read();  // Warning reported
        b |= x;
        b ^= bb;        // Warning reported
        b ^= i;         // Warning reported
        b ^= bv[1];     // Warning reported
    }
    
    void compound_bool_arithm() 
    {
        bool b = bool(s.read());
        bool bb = bool(s.read());
        sc_uint<1> a = s.read();
        sc_uint<12> x = s.read();
        sc_int<12> y = s.read();
        int i = s.read();
        
        b += bb;        // Warning reported
        b += a;    
        b += x;    
        b += y;         // Warning reported
        b += i;         // Warning reported
        b += s.read();  // Warning reported
        b += bv[1];     // Warning reported

        b *= bb;        // Warning reported
        b /= s.read();  // Warning reported
        b -= x;
        b %= bb;        // Warning reported
        b >>= i;
        b <<= bv[1];
    }    
    
    void compound_ref_compound() 
    {
        int k;
        int i = s.read();
        int l = 1;
        int& r = l;
        r += i;

        if (r) k = 1;
        l = 2;
        sct_assert_const(r == 2);
    }


    // Compound assignments SC types
    void sc_compound_assign() 
    {
        sc_uint<3> u = 3;
        sc_int<4> i = -2;
        sc_int<4> m2 = 1;
        i += 1;
        CHECK(i==-1);
        i -= m2;
        CHECK(i==-2);
        i *= 1;
        CHECK(i==-2);
        i /= u;
        CHECK(i==0);
        u %= k;         // Warning reported
        u |= 2;
        u &= 3;
        u ^= sc_uint<5>(4);
        u <<= k;
        u >>= sc_uint<4>(2+i);
    }
    
};

int sc_main(int argc, char* argv[])
{
    sc_signal<bool>  a{"a"};

    A a_mod{"a_mod"};
    a_mod.in(a);
    a_mod.out(a);

    sc_start();
    return 0;
}

