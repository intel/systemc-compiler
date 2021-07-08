/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"
#include "sctcommon/sct_assert.h"
#include <iostream>

using namespace sc_core;

// Concatenation operations for SC and C++ types

// sc_int< 8 > x ;
// sc_bigint< 8 > y ;
// x[3] = y[2] // Legal
// (x+x)[3] = 0 ; // Illegal, as x+x is promoted to a native C++ type
// (y+y)[3] = 0 ; // Legal as y+y is still a sc_bigint
// (y,y)[3] = 0 ; // Illegal as concatenation doesn’t support bitref

class A : public sc_module {
public:
    // Define variables
    sc_signal<bool> dummy;
    sc_uint<8> x1;
    sc_uint<8> x2;
    sc_uint<8> y1;
    sc_uint<8> y2;

    sc_in<sc_uint<32>> i1;
    sc_in<sc_uint<32>> i2;
    sc_in<sc_uint<32>> i3;
    sc_in<sc_uint<32>> i4;
    sc_out<sc_uint<32>> o1;
    sc_out<sc_uint<32>> o2;
    sc_out<sc_uint<32>> o3;
    sc_out<sc_uint<32>> o4;
    sc_signal<sc_int<32>> o5;
    sc_signal<sc_int<32>> o6;
    sc_signal<sc_int<32>> o7;
    sc_signal<sc_int<32>> o8;

    
    SC_CTOR(A) 
    {
        SC_METHOD(concat_lhs); sensitive << s3;
        // Error reported
        //SC_METHOD(incrdecr_lhs); sensitive << s3;
        SC_METHOD(cast_lhs); sensitive << s3;
        SC_METHOD(concat_return); sensitive << s3;
        SC_METHOD(concat_func_param); sensitive << s3;
        SC_METHOD(array_element_unknown); sensitive << s3;
        
        SC_METHOD(concat_cpp_bug); 
        sensitive << s1;
        
        SC_METHOD(concat_cpp); 
        sensitive << s1 << s2 << s3 << s4 << s5 << s6;

        SC_METHOD(concat_compl); 
        sensitive<< s1 << s2 << s3 << s4 << s5 << s6;
        
        SC_METHOD(bit_range_sel); 
        sensitive<< i1 << i2;

        SC_METHOD(bit_range_sel2); 
        sensitive<< i1;

        SC_METHOD(bit_range_sel3); 
        sensitive<< i1;
    }
    
    sc_signal<bool>     s1;
    sc_signal<char>     s2;
    sc_signal<int>      s3;
    sc_signal<unsigned> s4;
    sc_signal<long>     s5;
    sc_signal<sc_biguint<4>>  s6;
    
    // Concatenation assignment
    void concat_lhs() 
    {
        int k;
        sc_uint<1> a;
        sc_uint<2> b;

        concat(a,b) = s3.read();
        if (a) k = 1;
        if (b) k = 2;
        
        sc_int<10> c = 3;
        sc_int<20> d = 255;
        (c.bit(1), d.range(2, 1)) = s3.read();
        if (c && !d) k = 3;

        sc_biguint<70> e = 12;
        sc_bigint<80> f = 0;
        (e, f.range(69, 60)) = s3.read();
        if (!e.to_int()) k = 4;
        if (f == 0) k = 4;
    }

    // Prefix increment/decrement returns reference which can be in LHS
    void incrdecr_lhs() 
    {
        int k; 
        int i = 2;
        ++i = s3.read();
        
        if (i) {
            k = 1;
        }

        sc_uint<4> a = 1;
        --a = s3.read();
        if (a) {
            k = 2;
        }
        
        sc_bigint<16> b = 222;
        ++b = s3.read();
        if (b == 0) {
            k = 3;
        }
    }
    
    // Cast in LHS
    void cast_lhs() 
    {
        int k;
        sc_uint<4> a;
        unsigned i;

        // Error reported: LValueBitCast not supported
        //(int&)a = s3.read();
        //if (a) k = 1;
        //(int&)i = s3.read();
        //if (i) k = 1;
    }

    // Function returned result assigned to concatenation/range/bit/prefix
    sc_uint<4> retConcat(sc_uint<1> par1, sc_uint<3> par2) {
        par2++;
        return (par1, par2);
    }
    
    
    void concat_return() 
    {
        int k;
        sc_uint<1> a = 0;
        sc_uint<3> b = 0;
        (a, b) = retConcat(a, b);
        
        if (a) k = 1;
        if (b) k = 2;
        
        (a, b) = retConcat(a, b);
        if (a) k = 3;
        if (b) k = 4;

        
        sc_uint<6> c = 0;
        c.range(4,1) = retConcat(a, b);
        if (c) k = 6;
    }
    
    // Function reference parameter concatenation/range/bit/prefix
    template<class T>
    void refParam(T& par) {
        par = s3.read();
    }
    
    void concat_func_param() 
    {
        int k;
        sc_uint<1> a = 0;
        sc_uint<3> b = 0;
        int i = 0;
        refParam(a);
        refParam(b);
        refParam(i);

        if (a) k = 1;
        if (b) k = 2;
        if (i) k = 3;
        
        // Error reported:
//        refParam((a,b));
//        refParam(b[1]);
//        refParam(++i);
    }
    
    
    
    void array_element_unknown() 
    {
        int k;
        int arr[3] = {1,2,3};
        int arr2[2][3] = {{1,2,3}, {4,5,6}};
        int i = s3.read();
        arr[i] = 4;
        arr2[i][1] = 4;

        if (arr[0] == 0) k = 1;
        if (arr2[1][2] == 0) k = 2;
        sct_assert_unknown(k);
        
        sct_assert_unknown(arr[0]);
        sct_assert_unknown(arr[1]);
        sct_assert_unknown(arr[2]);

        sct_assert_unknown(arr2[0][0]);
        sct_assert_unknown(arr2[0][1]);
        sct_assert_unknown(arr2[0][2]);
        sct_assert_unknown(arr2[1][0]);
        sct_assert_unknown(arr2[1][1]);
        sct_assert_unknown(arr2[1][2]);
    }
    
    void operator_assign() 
    {
        int k;
        int arr[3] = {1,2,3};
        int arr2[2][3] = {{1,2,3}, {4,5,6}};
        int i = s3.read();
        arr[i] = 4;
        arr2[i][1] = 4;

        if (arr[0] == 0) k = 1;
        if (arr2[1][2] == 0) k = 2;
        
        sct_assert_unknown(arr[0]);
        sct_assert_unknown(arr[1]);
        sct_assert_unknown(arr[2]);
    }
    
    
// --------------------------------------------------------------------------    

    // Issue with unsized 1 in concatenation
    void concat_cpp_bug() 
    {
        bool b = 0;
        sc_uint<1> y = 1;
        sc_uint<16> z;
        
        z = (y, 1);
        cout << "z " << z << endl;
        sct_assert(z == 3);
        
        // Narrowing and widening
        z = (y, sc_uint<3>(1));
        z = (y, sc_uint<3>(22));
        
        z = (y, sc_uint<2>(sc_uint<3>(1)));
        z = (y, sc_uint<5>(sc_uint<3>(22)));
    }

    // Concat and non-intended comma for SC and CPP types mix
    void concat_cpp() 
    {
        bool b = s1.read();
        bool c = s1.read();
        int i = s3.read();
        sc_uint<1> y = s2.read();
        sc_biguint<33> by = s5.read();
        sc_uint<16> z;
        sc_biguint<40> bz;
        
        // Concat SC and integer/boolean
        z = concat(b, y);
        z = (b, sc_uint<1>(0));
        z = (y, 1, b);
        z = (y, 12, b);
        
        // Concat SC and integer, warning reported
        z = (y, i);
        z = (i, y);
        z = (s3, y); 
        z = (y, s3.read());
        z = (y, s4);
        z = (s5.read(), y, s4);
        
        // Non-intended comma, C++ compiler warning
//        z = (b, 1); 
//        z = (b, c);         
//        z = (i, c); 
//        z = (s1, s2);
//        z = (i++, s2);
//        z = (i, s2);
//        z = (s3, s4, s5, y);
        
        // Big types
        bz = concat(by, 11);
        bz = (by, s5);
        bz = (by, y);
        bz = (by, i);
        bz = (b, by);
        bz = (s4.read(), by);
        bz = (4, by, i);
    }
    
    // Multiple concat in one expression
    void concat_compl() 
    {
        bool b = s1.read();
        bool c = s1.read();
        int i = s3.read();
        sc_uint<1> y = s2.read();
        sc_uint<12> yy = s2.read();
        sc_biguint<33> by = s5.read();
        sc_uint<16> z;
        sc_biguint<40> bz;
        
        z = ((c, y), (s1, yy));
        z = ((i, yy), yy);
        
        bz = (yy++, by);
        bz = (by, sc_uint<5>(11));
    }
    
    void bit_range_sel() 
    {
        sc_bigint<4> tb; sc_bigint<4> xb;
        sc_int<4> t;  sc_int<4> x;

        y1=i1.read();
        y2=i2.read();

        (x1,x2) = (y1,y2);

        o1 = x1;
        o2 = x2;
    }
    
    void bit_range_sel2() 
    {
        sc_int<4> t;  sc_int<4> x;

        sc_uint<8> a = (t, x);
        // Type conversion required, else it is casted to boolean in Clang AST
        o3 = (t, sc_uint<4>(t*x)); 
        o4 = (sc_uint<4>(t),sc_uint<4>(t*x));
    }

    void bit_range_sel3() 
    {
        sc_bigint<4> tb; sc_bigint<4> xb;
        sc_int<4> t;  sc_int<4> x;
        t = 1;
        x = 2;
        tb = 1;
        xb = 2;

        o6 = (t, sc_uint<4>(tb*xb));
        o7 = (sc_uint<3>(x >> 1), t);
        o8 = (t, sc_biguint<5>(xb >> 1));
    }
    
};

class B_top : public sc_module {
public:
    sc_signal<sc_uint<32>> i1;
    sc_signal<sc_uint<32>> i2;
    sc_signal<sc_uint<32>> i3;
    sc_signal<sc_uint<32>> i4;
    sc_signal<sc_uint<32>> o1;
    sc_signal<sc_uint<32>> o2;
    sc_signal<sc_uint<32>> o3;
    sc_signal<sc_uint<32>> o4;

    A a_mod{"a_mod"};
    
    SC_CTOR(B_top) {
        a_mod.i1(i1);
        a_mod.i2(i2);
        a_mod.i3(i3);
        a_mod.i4(i4);
        a_mod.o1(o1);
        a_mod.o2(o2);
        a_mod.o3(o3);
        a_mod.o4(o4);
    }
};

int sc_main(int argc, char *argv[]) 
{
    B_top bmod{"b_mod"};
    sc_start();
    return 0;
}

