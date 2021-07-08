/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"


// Implicit and explicit cast operations for variables and constants, 
// including multiple casts
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    
    int                 m;
    int                 k;

    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;
    const int M1 = 42;
    const int M2 = 44;
    const int* pm = &M1;
    
    sc_signal<bool> dummy{"dummy"};
    sc_signal<sc_uint<4>> s;

    SC_CTOR(A)
    {
        *(const_cast<int*>(pm)) = 43;
        const_cast<int&>(M2) = 45;
        // #266
        SC_METHOD(cast_to_unsigned); sensitive << is << xs << us << bs;
        
        SC_METHOD(const_cast_for_variable); sensitive << dummy;
        
        SC_METHOD(cond_const); sensitive << dummy;
        SC_METHOD(bool_cast); sensitive << dummy;
        SC_METHOD(const_bool_type_cast); sensitive << dummy;
        SC_METHOD(const_cpp_type_cast); sensitive << dummy;
        SC_METHOD(var_cpp_type_cast); sensitive << dummy;
        SC_METHOD(const_cpp_ref_impl_cast); sensitive << dummy;
        SC_METHOD(const_cpp_ref_expl_cast); sensitive << dummy;
        SC_METHOD(var_cpp_expl_cast); sensitive << dummy;
        
        SC_METHOD(const_sc_type_cast); sensitive << dummy;
        SC_METHOD(var_sc_type_cast); sensitive << dummy;
        SC_METHOD(multi_sc_type_cast); sensitive << s;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    sc_signal<int> is;
    sc_signal<unsigned> us;
    sc_signal<sc_uint<15>> xs;
    sc_signal<sc_biguint<15>> bs;
    void cast_to_unsigned() 
    {
        int i = -11;
        unsigned u = 12;
        sc_uint<15> x = 12;
        sc_biguint<15> bx = 12;
        sc_int<15> y = -11;
        sc_bigint<15> by = -11;
        long res;
        
        // TODO: Fix me, signed/unsigned in SV required?    (#266.2)
        res = (unsigned)i;
        res = (unsigned)y;
        res = (int)x;
        res = (int)u;
        
        // Cast to unsigned in binary
        // TODO: Fix me. Must be unsigned arithmetic!!!     (#266.3)
        res = x + (unsigned)i;                    // Error

        // Cast to signed in binary
        // TODO: fix me, signed required for both arguments (#266.3)
        res = int(u) + x;                         // Error
        res = u + int(x);                         // Error
        
        // TODO: Fix me. `signed required here for @x
        res = x.to_int() + x;                     // Error
        res = x.to_int() + i;                     // OK
        res = xs.read().to_int() + xs.read();     // Error
        res = xs.read().to_int() + is.read();     // OK
        
        // Signed casted to signed
        res = sc_int<12>(i) + u;                  // OK
        res = sc_int<12>(y) + x;                  // OK  
        res = sc_int<12>(is.read()) + us.read();  // OK
        res = sc_int<12>(is.read()) + xs.read();  // OK
        
        res = x + i;            // OK
        
        // Unintended signed arithmetic for @sc_biguint    (#266.4)
        res = 10 - u;
        res = 10 - x;
        res = u - 10;
        res = x - 10;
        res = x - u;

        bx = 10 + bx;               // Error?
        bx = 10 - bs.read();        // Error?    

//        res = bx - 10;
//        res = 10 - bx;
//        res = x - bx;
//        res = bx - x;
//        res = u - bx;
//        res = bx - u;
    }
    
    // @const_cast<> to remove constantness
    sc_signal<int> r1;
    void const_cast_for_variable() {
        CHECK (M1 == 43);
        CHECK (M2 == 45);
        
        // That is OK
        int i = *(const_cast<int*>(pm));
        // Error reported, M1 is @localparam cannot be changed
        //*(const_cast<int*>(pm)) = 46;
        
        auto j = (const int)i;
        r1 = j+1;
    }
    
    void cond_const() {
        bool b;
        b = (CONST_Z) ? 1 : 2;
        CHECK(b);
        b = (CONST_Z) ? bool(1) : bool(2);
        
        unsigned char c;
        c = (CONST_A) ? 258 : 259;
        CHECK(c == 2);
        c = (CONST_A) ? (unsigned char)258 : (unsigned char)259;
    }
    
    void bool_cast() {
        bool b = 1;
        b = !b;

        int i = 2;
        i = ~i;
        
        b = i;
    }

    // Implicit and explicit cast for bool type
    void const_bool_type_cast() {
        bool b;
        // Implicit cast
        unsigned char c;
        c = 2;
        b = c;
        CHECK(b);
        
        b = 257;
        CHECK(b);
        
        c = 0;
        b = c;
        CHECK(!b);
        
        // Explicit cast
        unsigned int i;
        i = 65536;
        b = (bool)i;
        CHECK(b);
        
        b = (bool)65536;
        CHECK(b);
        
        i = 0;
        b = (bool)i;
        CHECK(!b);
    }
    
    // Implicit and explicit cast for CPP types
    void const_cpp_type_cast() {
        // Implicit cast
        unsigned char c;
        c = 257;
        CHECK(c == 1);
        
        unsigned short s = 257;
        c = s;
        CHECK(c == 1);
        
        unsigned int i;
        unsigned long l = (unsigned long)1 << 32;
        CHECK(l == 4294967296);
        i = l + 1;
        CHECK(i == 1);
        
        // Explicit cast
        i = (unsigned char)257;
        CHECK(i == 1);
        i = (unsigned short)65537;
        CHECK(i == 1);
        
        i = (unsigned char)s + 1;
        CHECK(i == 2);
    }
    
    void var_cpp_type_cast() {
        // Implicit cast
        unsigned char c;
        unsigned short s;
        unsigned int i;
        unsigned long l = 0xAAAABBBBCCCCDDEEULL;

        i = l;
        s = i;
        c = s;
        
        CHECK(l == 0xAAAABBBBCCCCDDEEULL);
        CHECK(i == 0xCCCCDDEE);
        CHECK(s == 0xDDEE);
        CHECK(c == 0xEE);
    }

    // Cast for CPP types in references
    void const_cpp_ref_impl_cast() {
        unsigned int i = 65537;
        unsigned short s = 257;
        unsigned short& rs = s; 
        
        unsigned char c = rs;
        CHECK(c == 1);
        CHECK(s == 257);
        
        c = rs + 1;
        CHECK(c == 2);
    }

    void const_cpp_ref_expl_cast() {
        unsigned int i = 65537;
        unsigned short s = 257;
        unsigned short& rs = s; 
        
        unsigned int j = (unsigned char)rs;
        CHECK(j == 1);
        CHECK(s == 257);
        
        j = (unsigned char)rs + 1;
        CHECK(j == 2);
    }
    
    void var_cpp_expl_cast() {
        unsigned int u;
        unsigned short s;
        sc_uint<33> ux = 0x1C0000000; 
        sc_biguint<4> bu; 
        sc_int<4> ix; 
        sc_bigint<4> bi; 
        
        int i;
        i = ux.to_int();
        cout << hex << " i " << i << endl;
        CHECK(i == 0xC0000000);
        i = ux.to_int()+1;
        cout << hex << " i " << i << endl;
        CHECK(i == 0xC0000001);
        
        i = (int)s + 1;
        i = (int)u + 1;
        i = (int)ux + ux.to_int() + bu.to_int();
        i = ux.to_int();
        i = bu.to_int();
        i = (int)ix + ix.to_int() + bi.to_int();
        
        i = ix.to_int();
        i = ix.to_long();
        i = ix.to_int64();
        i = ix.to_uint();
        i = ix.to_ulong();
        i = ix.to_uint64();

        i = bi.to_int();
        i = bi.to_long();
        i = bi.to_int64();
        i = bi.to_uint();
        i = bi.to_ulong();
        i = bi.to_uint64();
    }
    
    // Implicit and explicit cast for SC types
    void const_sc_type_cast() {
        sc_uint<4> x = 12;
        sc_uint<3> y = x;
        CHECK(y == 4);
        
        sc_int<5> sx = -7;
        sc_int<3> sy = sx;
        //cout << "sy " << sy << endl;
        CHECK(sy == 1);    // It is really equals to 1
        
        sc_uint<5> z = (sc_uint<3>)x;
        CHECK(z == 4);
        
        z = (sc_uint<3>)(x+1);
        CHECK(z == 5);

        z = (sc_uint<3>)(13);
        CHECK(z == 5);

        z = (sc_uint<2>)15;
        CHECK(z == 3);
        
        unsigned int i = 14;
        z = (sc_uint<2>)i;
        CHECK(z == 2);
        
        sc_int<5> sz;
        sz = (sc_uint<3>)sx;
        cout << "sz " << sz << endl;
        CHECK(sz == 1);
        
        sz = (sc_uint<3>)(-13);
        cout << "sz " << sz << endl;
        CHECK(sz == 3);
    }    
    
    void var_sc_type_cast() {
        // Implicit cast
        sc_uint<8> u1;
        sc_uint<16> u2;
        sc_biguint<32> b1;
        sc_biguint<48> b2; 
        unsigned int i = 0xAAAABBCD;
        unsigned long l = 0xAAAABBBBCCCCDDEEULL;

        u1 = i; 
        CHECK(u1 == 0xCD);
        u1 = l;
        CHECK(u1 == 0xEE);
        u2 = i; 
        CHECK(u2 == 0xBBCD);
        u2 = l;
        CHECK(u2 == 0xDDEE);
        b1 = i; 
        CHECK(b1 == 0xAAAABBCD);
        b1 = l;
        CHECK(b1 == 0xCCCCDDEE);
        b2 = i; 
        CHECK(b2 == 0xAAAABBCD);
        b2 = l;
        CHECK(b2 == 0xBBBBCCCCDDEEULL);
    }

    // Multiple casts for SC types
    static const unsigned CC = 42;
    static const int SC = -42;
    void multi_sc_type_cast() {
        sc_uint<4> x = s;
        sc_uint<8> y = (sc_uint<8>)((sc_uint<6>)x);
        y = (sc_uint<6>)((sc_uint<8>)x);
        y = (sc_uint<6>)((sc_uint<2>)x);
        y = (sc_uint<2>)((sc_uint<3>)x);
        
        sc_uint<16> z = ((sc_uint<8>)((sc_uint<3>)y), 
                         (sc_uint<8>)((sc_uint<3>)0x11));
        z = ((sc_uint<8>)((sc_uint<3>)y), 
              (sc_uint<8>)((sc_uint<3>)CC));
        
        sc_int<16> sz = ((sc_int<8>)(-(sc_int<3>)y), 
                         (sc_int<8>)((sc_int<3>)-0x11));
        sz = ((sc_int<8>)((sc_int<3>)-y), 
              (sc_int<8>)((sc_int<3>)SC));
    }
    
};

class B_top : public sc_module 
{
public:
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};
    sc_signal<bool>  c{"c"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

