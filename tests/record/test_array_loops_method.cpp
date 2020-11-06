/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Record Array tests: Loop and if/else using signal as indices

class A : public sc_module {
public:

    sc_in<sc_uint<2>>         a{"a"};
    sc_signal<sc_uint<2>>         as{"as"};
    sc_in<sc_int<2>>         as_i{"as_i"};

    sc_signal<bool>     s{"s"};


    SC_CTOR(A) {

        SC_METHOD(record_var_loops);
        sensitive << a << as << as_i << s;
        SC_METHOD(record_var_loops_binary);
        sensitive << a << as << as_i << s;
        //SC_METHOD(record_var_loops_arith);
        //sensitive << a << as << as_i << s;
    }

    // Function with records
    struct Simple {
        int a;
        sc_uint<2> b;
    };
    struct Simple2 {
        sc_uint<16> a;
        sc_uint<32> b;
        sc_uint<32> c;
        sc_uint<32> d;
        sc_uint<32> e;
        sc_uint<32> f;
        sc_uint<32> g;
        sc_uint<32> h;
        };
    struct Simple3 {
        sc_uint<16> a;
        sc_uint<32> b;
        sc_uint<32> c;
        sc_uint<32> d;
        sc_uint<32> e;
        sc_uint<32> f;
        sc_uint<32> g;
        sc_uint<32> h;
        };
    Simple rec[16];
    Simple2 rec2a[12];
    Simple2 rec2b[12];
    Simple3 rec3a[8];
    Simple3 rec3b[8];

    void record_var_loops()
    {
        int num1 = 2;
        int num2 = 2;

        rec[0].a = 0;
        rec[1].a = 1;
        rec[num1++].a = num2++;
        rec[num1++].a = num2++;
        rec[num1++].a = num2++;
        rec[num1++].a = num2++;
        for (int p=0; p<6; p++) {
            sct_assert_const(rec[p].a==p);
        }

        num1=2;
        if (num1==2) {
            rec[0].a = 0;
        } else {
            rec[0].a = 1;
        }
        if (num2==1) {
            rec[1].a = 1;
        } else {
            rec[1].a = 5;
        }

        sct_assert_const(rec[0].a==0);
        sct_assert_const(rec[1].a==5);

        for (int i = 0; i < rec[a.read()].a-10; i++) {
            rec[as.read()+i].a = 30+i;
        }
        rec[as_i.read()].a=35;
        rec[s.read()].a=40;
    }

    void record_var_loops_binary()
    {
        int num1 = 2;
        int num2 = 2;

        rec2a[0].a = 0;
        rec2a[1].a = 1;
        rec2a[num1++].a = num2++;
        rec2a[num1++].a = num2++;
        rec2a[num1++].a = num2++;
        rec2a[num1++].a = num2++;

        int num1a = 2;

        rec2b[0].a = rec2a[0].a & rec2a[1].a;
        rec2b[1].a = rec2a[1].a | rec2a[2].a;
        rec2b[num1a++].a = rec2a[2].a ^ rec2a[3].a;
        rec2b[num1a++].a = !(rec2a[2].a ^ rec2a[3].a);
        rec2b[num1a++].a = rec2a[2].a[1] || rec2a[3].a;
        rec2b[num1a++].a = rec2a[2].a[2] && rec2a[4].a;
        
        for (int p=0; p<6; p++) {
            rec2b[p].c=rec2a[p].a | rec2b[p].a;
            rec2b[p].d=rec2a[p].a & rec2b[p].a;
            rec2b[p].e=rec2a[p].a ^ rec2b[p].a;
            rec2b[p].f=!(rec2a[p].a);
            rec2b[p].g=rec2a[p].a || rec2b[p].a;
            rec2b[p].h=rec2a[p].a && rec2b[p].a;
        }
        
        sct_assert_const(rec2b[0].c == 0);
        sct_assert_const(rec2b[0].d == 0);
        sct_assert_const(rec2b[0].e == 0);
        sct_assert_const(rec2b[0].f == 1);
        sct_assert_const(rec2b[0].g == 0);
        sct_assert_const(rec2b[0].h == 0);
        sct_assert_const(rec2b[1].c == 3);
        sct_assert_const(rec2b[1].d == 1);
        sct_assert_const(rec2b[1].e == 2);
        sct_assert_const(rec2b[1].f == 0);
        sct_assert_const(rec2b[1].g == 1);
        sct_assert_const(rec2b[1].h == 1);
        cout << "rec2b[2].c = " << rec2b[2].c << endl;

        // Too complicated arithmetic cannot be statically evaluated
        //sct_assert_const(rec2b[2].c == 3);
    }
    
};

class B_top : public sc_module {
public:
    sc_signal<sc_uint<2>>  a{"a"};
    sc_signal<sc_uint<2>>  as{"as"};
    sc_signal<sc_int<2>>  as_i{"as_i"};


    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.as_i(as_i);

    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

