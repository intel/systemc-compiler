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

// for statement with SC and special C++ type of loop counter
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

    SC_CTOR(A) {
        SC_METHOD(sc_type_for_uint);
        sensitive << a;
        SC_METHOD(sc_type_for_bigint);
        sensitive << a;
        SC_METHOD(sc_type_for_biguint);
        sensitive << a;
        SC_METHOD(sc_type_for_long);
        sensitive << a;
        SC_METHOD(sc_type_for_ulong);
        sensitive << a;
        SC_METHOD(sc_type_for_short);
        sensitive << a;
        SC_METHOD(sc_type_for_ushort);
        sensitive << a;
        SC_METHOD(sc_type_for_uint128);
        sensitive << a;
        SC_METHOD(sc_type_for_int128);
        sensitive << a;

    }

    static const unsigned PORT_NUM = 1;
    sc_signal<bool>       core_req[PORT_NUM];

    void sc_type_for_uint()
    {
        int a[8];
        for (sc_uint<4> i = 0; i < 8; i++) {
            a[i] = i;
            cout << "a[" << i << "] is " << a[i] << endl;
//            #10 $display("The value of a[%d] is: %d",i,a_1[i]) ;

        }
    }
    void sc_type_for_bigint()
    {
        int a2[10];
        unsigned int a_u[20];
        long a_l[20];
        unsigned long a_ul[20];
        // Unsupported SC object method to_long, to_int, to_uint, to_ulong
        for (sc_bigint<10> i2 = 0; i2 < 10; i2++) {
            a2[i2.to_int()] = i2.to_int();
        }

        for (sc_bigint<64> i3 = 0; i3 < 10; i3++) {
            a_u[i3.to_uint()] = i3.to_uint();
        }
        for (sc_bigint<40> i3 = 0; i3 < 10; i3++) {
            a_l[i3.to_long()] = i3.to_long();
        }
        for (sc_bigint<40> i3 = 0; i3 < 10; i3++) {
                a_ul[i3.to_ulong()] = i3.to_ulong();
        }
        sc_bigint<40> a_bint;
        for (sc_bigint<115> i4 = 0; i4 < 10; i4++) {
            a_bint[i4.to_int()] = i4.or_reduce();
        }

    }

    void sc_type_for_biguint()
    {
        int abuint2[10];
        unsigned int abuint_u[20];
        long abuint_l[20];
        unsigned long abuint_ul[20];

        for (sc_biguint<40> ibu2 = 0; ibu2 < 10; ibu2++) {
            abuint2[ibu2.to_int()] = ibu2.to_int();
        }

        for (sc_biguint<45> ibu3 = 0; ibu3 < 10; ibu3++) {
            abuint_u[ibu3.to_uint()] = ibu3.to_uint();
        }
        for (sc_biguint<140> ibu3 = 0; ibu3 < 10; ibu3++) {
            abuint_l[ibu3.to_long()] = ibu3.to_long();
        }
        for (sc_biguint<65> ibu3 = 0; ibu3 < 10; ibu3++) {
                abuint_ul[ibu3.to_ulong()] = ibu3.to_ulong();
        }
        sc_biguint<40> a_buint;
        for (sc_biguint<5> ibu4 = 0; ibu4 < 10; ibu4++) {
            a_buint[ibu4.to_int()] = ibu4.or_reduce();
        }

    }

    void sc_type_for_long()
    {
    	sc_uint<10> a;
    	for (long long_x = 0; long_x < 10; long_x++)
        {
            a[long_x] = long_x;
        }
    }

    void sc_type_for_ulong()
    {
        sc_uint<10> a;
        for (unsigned long ulong_x = 0; ulong_x < 10; ulong_x++)
        {
            a[ulong_x] = ulong_x;
        }
    }
    
    void sc_type_for_short()
    {
        sc_uint<10> a;
        for (short short_x = 0; short_x < 10; short_x++)
        {
            a[short_x] = short_x;
        }
    }
    
    void sc_type_for_ushort()
    {
        sc_uint<10> a;
        for (unsigned short ushort_x = 0; ushort_x < 10; ushort_x++)
        {
            a[ushort_x] = ushort_x;
        }
    }
    
    void sc_type_for_int128()
    {
        sc_uint<10> a;
        for (__int128_t int128_x = 0; int128_x < 10; int128_x++)
        {
            a[int128_x] = int128_x;
        }
    }
    
    void sc_type_for_uint128()
    {
        sc_uint<10> a;
        for (__uint128_t uint128_x = 0; uint128_x < 10; uint128_x++)
        {
            a[uint128_x] = uint128_x;
        }
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

int  sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

