/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>

using namespace sc_core;

// while statement with various types
class A : public sc_module
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;

    int                 m;
    int                 k;
    int                 n;
    int*                q;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(while_sc_int_type); sensitive << a;
        SC_METHOD(while_sc_uint_type); sensitive << a;
        SC_METHOD(while_sc_bigint_type); sensitive << a;
        SC_METHOD(while_sc_biguint_type); sensitive << a;
        SC_METHOD(while_long_type); sensitive << a;
        SC_METHOD(while_ulong_type); sensitive << a;
        SC_METHOD(while_short_type); sensitive << a;
        SC_METHOD(while_ushort_type); sensitive << a;
        SC_METHOD(while_mixed_type); sensitive << a;

    }

    // sc_int type
    void while_sc_int_type() {
        int k = 0;
        sc_int<3> i = 0;
        sc_int<4> j = 0;
        while ((i < 2) && (j<=3) || (k == 5)) {
            k = k + 1;
            i++;
            j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
        sct_assert_const(k == 2);
    }
    
    // sc_uint type
    void while_sc_uint_type() {
        int k = 0;
        sc_uint<5> i = 0;
        sc_uint<6> j = 0;
        while ((i < 2) && (j<=3) || (k == 5)) {
                i++;
                j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
    }
    
    void while_sc_bigint_type() {
        int k = 0;
        sc_bigint<5> i = 0;
        sc_bigint<6> j = 0;
        while ((i < 2) && (j<=3) || (k == 5)) {
                i++;
                j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
    }
    
    void while_sc_biguint_type() {
        int k = 0;
        sc_biguint<7> i = 0;
        sc_biguint<8> j = 0;
        while ((i < 2) && (j<=3) || (k == 5)) {
                i++;
                j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
    }
    
    void while_long_type() {
        int k = 0;
        long i = 0;
        long j = 0;
        while ((i < 2) && (j<=3) || (k == 5)) {
                i++;
                j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
        //sct_assert_const(k == 2);
    }
    
    void while_ulong_type() {
        int k = 0;
        unsigned long i = 0;
        unsigned long j = 0;
        while ((i < 2) && (j<=3) || (k == 5)) {
                i++;
                j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
    }
    
    void while_short_type() {
        int k = a.read();
        short i = 0;
        short j = 0;
        while ((i < 2) && (j<=3) || (k == 5)) {
                i++;
                j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
        //sct_assert_const(k == 2);
    }

    void while_ushort_type() {
        int k = a.read();
        unsigned short i = 0;
        unsigned short j = 0;
         while ((i < 2) && (j<=3) || (k == 5)) {
                i++;
                j++;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
    }
    
    void while_mixed_type() {
        int k = a.read();
        unsigned short i = 0;
        unsigned short j = 0;
        short l = 1;
        long m = 3;
        unsigned long n = 2;
        sc_int<10> o = 4;
        sc_uint<20> p = 5;
        sc_bigint<32> q = 6;
        sc_biguint<63> r = 18;

        while ((i < 2) && (j<=3) || (k == 5) && (l==0) && (o) || (p) && (q.or_reduce()) || (r.or_reduce()) ) {
                i++;
                j++;
                l-=1;
                m-=2;
                n-=1;
                o--;
                p+=2;
                q-=1;
                r-=2;
        }
        cout << "i = " << i << ". j = " << j << ". k = " << k << endl;
    }
};

class B_top : public sc_module
{
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

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

