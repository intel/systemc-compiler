/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"


// Implicit and explicit cast operations for variables and constants
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_signal<bool>     s{"s"};
    sc_signal<bool>*    ps;
    
    int                 m = 1;
    int                 k = 2;
    int*                p;
    int*                p1;
    int*                p2;
    int*                q;
    sc_uint<5>*         px;

    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        p = sc_new<int>();
        p1 = sc_new<int>();
        p2 = sc_new<int>();
        q = nullptr;
        ps = new sc_signal<bool>("ps");
        px = sc_new<sc_uint<5>>();
        
        SC_METHOD(bool_arithmetic); sensitive << dummy;
        
        SC_METHOD(test_bool_to_bool); sensitive << a << s << b << *ps;
        SC_METHOD(test_bool_unary); sensitive << dummy;
        SC_METHOD(test_sc_to_bool); sensitive << dummy;
        SC_METHOD(test_ptr_comp); sensitive << dummy;
        SC_METHOD(test_int_comp); sensitive << dummy;
        SC_METHOD(test_sc_comp); sensitive << a;
        
        SC_METHOD(test_bool1); sensitive << a;
        SC_METHOD(test_bool2); sensitive << dummy;
        SC_METHOD(test_bool4); sensitive << dummy;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    void bool_arithmetic() 
    {
        int res;
        
        bool b = true;
        unsigned u = 42;
        int i = -42;
        sc_uint<16> ux = 43;
        sc_int<16>  ix = -43;
        sc_biguint<16> ub = 44;
        sc_bigint<16>  ib = -44;
        
        res = b + b*b;
        CHECK(res == 2);
        res = b + 1;
        CHECK(res == 2);
        res = u + b;
        CHECK(res == 43);
        res = i + b;
        CHECK(res == -41);
        res = ux + b;
        CHECK(res == 44);
        res = ix + b;
        CHECK(res == -42);
        res = ub.to_uint64() + b;
        CHECK(res == 45);
        res = ib.to_int64() + b;
        CHECK(res == -43);

        res = -b + i;
        CHECK(res == -43);
    }

    // Try to find not required (extra) conversion
    void test_bool_to_bool() {
        bool b1 = a;
        b1 = a.read();
        bool b2 = b1;
        b2 = b1;
        
        b = b1;
        b.write(b1);
        b = a;
        b = a.read();
        b.write(a);
        b.write(a.read());
        
        s = b;
        s = b1;
        b = s;
        b = s.read();
        
        ps->write(b1);
        *ps = b;
        b = *ps;
        b = ps->read();
    }
    
    // Cast to boolean of unary expressions
    void test_bool_unary() {
        unsigned i = 2;
        bool b;
        b = +i;
        b = -i;
        b = i++;
        b = --i;
    }
    
    // Cast to boolean of unary expressions
    void test_sc_to_bool() {
        bool b1;
        sc_uint<3> x;
        b1 = x.bit(1);
        
        b1 = x.range(2,1);
        b1 = x;
        
        b1 = px->bit(1);
        b1 = px->range(2,1);
        b1 = *px;
        
        b1 = *px + x;
        
        s = x.bit(1);
        s = x.range(2,1);
        s = x;

        *ps = x.bit(1);
        ps->write(x.range(2,1));
        *ps = x;
    }
    
    // Pointer comparison to boolean conversion
    void test_ptr_comp() {
        bool b;
        b = p1;
        b = !p1;
        b = q;
        b = !q;
        
        b = p1 == nullptr;
        b = p1 != nullptr;

        b = p1 == q;
        b = p1 != q;

        b = p1 == p1;
        b = p1 != p1;
        b = q == q;
        b = q != q;
    }

    // Integer comparison to boolean conversion
    void test_int_comp() {
        bool b1;
        int i;
        sc_uint<4> x;
        
        b1 = i == 0;
        b1 = x > 1;
        b1 = i != x;
        
        b = i < 3;
        b.write(x >= i);
        
        s = x == i;
        s.write(*p2 > 0);
        
        *ps = x != 0;
        ps->write(i == *p2);
    }

    // SC type comparison with boolean
    void test_sc_comp() {
        bool b1;
        sc_bigint<4> x;
        sc_bigint<12> ux;
        
        bool b2 = b1==ux;
        b2 = b1==x;
    }
    
    // Integer to boolean conversion
    void test_bool1() {
        unsigned i = 2;
        bool b1;
        b1 = 2;
        b1 = i;
        b1 = (bool)i;
        b1 = i;
        b1 = i + 1;
        b1 = (bool)i + 1;
        b1 = (a.read()) ? m : m == i;
        b1 = (m) ? (bool)++m : m > 0;
    }

    // SC types to boolean
    void test_bool2() {
        sc_uint<4> x = 6;
        bool b;
        b = x.bit(2);
        b = x.range(3,1);
        b = x;
        b = !x;
        
        b = !x.bit(2);
        b = !x.range(3,1);
        b = x + x.range(3,1);
    }
    
    // Pointer to boolean conversion in conditions
    void test_bool4() {
        unsigned i;
        if (!p) {i = 0;}
        if (p) {i = 1;}
        if (!p) {i = 0;}
        sct_assert_const(i == 1);
        if (p != nullptr) {i = 2;}
        sct_assert_const(i == 2);
        
        if (q) {i = 0;}
        if (!q) {i = 3;}
        sct_assert_const(i == 3);
        if (q == nullptr) {i = 4;}
        sct_assert_const(i == 4);
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

