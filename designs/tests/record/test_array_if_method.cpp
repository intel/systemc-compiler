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

// Record Array tests: IF/else using signal as indices

class A : public sc_module {
public:

    sc_in<sc_uint<2>>         a{"a"};
    sc_signal<sc_uint<2>>         as{"as"};
    sc_in<sc_int<2>>         as_i{"as_i"};

    sc_signal<bool>     s{"s"};


    SC_CTOR(A) {

        SC_METHOD(record_var_if);
        sensitive << a << as << as_i << s;
        SC_METHOD(record_var_if_bitwise);
        sensitive << a << as << as_i << s;
        SC_METHOD(record_var_if_arith);
        sensitive << a << as << as_i << s;


    }

//-----------------------------------------------------------------------------
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

    void record_var_if()
    {
        int num1 = 0;
        int num2 = 1;


        if (num1==0) {
            rec[num1].a = 0;
        } else {
            rec[num1].a = 1;
        }
        if (num2!=1) {
            rec[num2].a = 0;
        } else {
            rec[num2].a = 1;
        }

        //cout << "rec[0] = "<< rec[0].a << endl;
        //cout << "rec[1] = "<< rec[1].a << endl;
        sct_assert_const(rec[0].a==0);
        sct_assert_const(rec[1].a==1);

        if (as_i.read()==0) {
            rec[as_i.read()].a=35;
            rec[s.read()].a=45;

        } else {
            rec[as_i.read()].a=25;
            rec[s.read()].a=50;

        }
    }

    void record_var_if_bitwise()
    {
        int num1 = 2;
        int num2 = 2;
        int num3 = 2;

        rec2a[0].a = 15;
        rec2a[1].a = 30;
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


        if (rec2b[0].a==0) {
            rec2b[num3].c = rec2a[0].a | rec2a[1].a ;
            rec2b[num3].d = rec2a[0].a & rec2a[1].a ;
            rec2b[num3].e = rec2a[0].a || rec2a[1].a ;
            rec2b[num3].f = rec2a[0].a && rec2a[1].a ;
            rec2b[num3].g = rec2a[0].a ^ rec2a[1].a ;
            rec2b[num3].h = !(rec2a[0].a);

        } else {
            rec2b[num3].c = rec2a[0].a & rec2a[1].a ;
            rec2b[num3].d = rec2a[0].a | rec2a[1].a ;
            rec2b[num3].e = rec2a[0].a && rec2a[1].a ;
            rec2b[num3].f = rec2a[0].a || rec2a[1].a ;
            rec2b[num3].g = !(rec2a[0].a);
            rec2b[num3].h = rec2a[0].a ^ rec2a[1].a ;
        }
        if (rec2b[num3].c!=1) {
            rec2b[num3+1].c = rec2a[0].a | rec2a[1].a ;
            rec2b[num3+1].d = rec2a[0].a & rec2a[1].a ;
            rec2b[num3+1].e = rec2a[0].a || rec2a[1].a ;
            rec2b[num3+1].f = rec2a[0].a && rec2a[1].a ;
            rec2b[num3+1].g = rec2a[0].a ^ rec2a[1].a ;
            rec2b[num3+1].h = !(rec2a[0].a);
        } else {
            rec2b[num3+1].c = rec2a[0].a & rec2a[1].a ;
            rec2b[num3+1].d = rec2a[0].a | rec2a[1].a ;
            rec2b[num3+1].e = rec2a[0].a && rec2a[1].a ;
            rec2b[num3+1].f = rec2a[0].a || rec2a[1].a ;
            rec2b[num3+1].g = !(rec2a[0].a);
            rec2b[num3+1].h = rec2a[0].a ^ rec2a[1].a ;

        }

        sct_assert_const(rec2b[2].c == 14);
        sct_assert_const(rec2b[2].d == 31);
        sct_assert_const(rec2b[2].e == 1);
        sct_assert_const(rec2b[2].f == 1);
        sct_assert_const(rec2b[2].h == 17);
        sct_assert_const(rec2b[3].c == 31);
        sct_assert_const(rec2b[3].d == 14);
        sct_assert_const(rec2b[3].e == 1);
        sct_assert_const(rec2b[3].f == 1);
        sct_assert_const(rec2b[3].g == 17);

    }
    void record_var_if_arith()
    {
        int num1 = 2;
        int num2 = 2;
        int num3 = 2;

        rec3a[0].a = 15;
        rec3a[1].a = 30;
        rec3a[num1++].a = num2++;
        rec3a[num1++].a = num2++;
        rec3a[num1++].a = num2++;
        rec3a[num1++].a = num2++;

        int num1a = 2;

        rec3b[0].a = rec3a[0].a + rec3a[1].a;
        rec3b[1].a = rec3a[1].a - rec3a[2].a;
        rec3b[num1a++].a = rec3a[2].a * rec3a[3].a;
        rec3b[num1a++].a = (rec3a[1].a / rec3a[3].a);

        if (rec3b[0].a==0) {
            rec3b[num3].c = rec3a[0].a + rec3a[1].a ;
            rec3b[num3].d = rec3a[1].a - rec3a[0].a ;
            rec3b[num3].e = rec3a[0].a * rec3a[1].a ;
            rec3b[num3].f = rec3a[0].a / rec3a[1].a ;
            rec3b[num3].g = rec3a[1].a % rec3a[0].a ;

        } else {
            rec3b[num3].c = rec3a[1].a - rec3a[0].a ;
            rec3b[num3].d = rec3a[0].a + rec3a[1].a ;
            rec3b[num3].e = rec3a[1].a / rec3a[0].a ;
            rec3b[num3].f = rec3a[2].a * rec3a[3].a ;
            rec3b[num3].g = rec3a[1].a % rec3a[0].a ;
        }
        if (rec3b[num3].c!=1) {
            rec3b[num3+1].c = rec3a[1].a - rec3a[0].a ;
            rec3b[num3+1].d = rec3a[0].a + rec3a[1].a ;
            rec3b[num3+1].e = rec3a[1].a / rec3a[0].a ;
            rec3b[num3+1].f = rec3a[2].a * rec3a[3].a ;
            rec3b[num3+1].g = rec3a[1].a % rec3a[0].a ;
        } else {
            rec3b[num3+1].c = rec3a[0].a + rec3a[1].a ;
            rec3b[num3+1].d = rec3a[1].a - rec3a[0].a ;
            rec3b[num3+1].e = rec3a[0].a * rec3a[1].a ;
            rec3b[num3+1].f = rec3a[0].a / rec3a[1].a ;
            rec3b[num3+1].g = rec3a[1].a % rec3a[0].a ;


        }


        sct_assert_const(rec3b[2].c == 15);
        sct_assert_const(rec3b[2].d == 45);
        sct_assert_const(rec3b[2].e == 2);
        sct_assert_const(rec3b[2].f == 6);
        sct_assert_const(rec3b[3].c == 15);
        sct_assert_const(rec3b[3].d == 45);
        sct_assert_const(rec3b[3].e == 2);
        sct_assert_const(rec3b[3].f == 6);


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

