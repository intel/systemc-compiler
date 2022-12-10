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

// Record (structure/class) tests with variables in indices

class A : public sc_module {
public:

    sc_in<sc_uint<2>>         a{"a"};
    sc_signal<sc_uint<2>>         as{"as"};
    sc_in<sc_int<2>>         as_i{"as_i"};

    sc_signal<bool>     s{"s"};


    SC_CTOR(A) {

        SC_METHOD(record_var_indices);
        sensitive << a << as << as_i << s;

        SC_METHOD(record_var_indices_arith);
        sensitive << a << as << as_i << s;

        SC_METHOD(record_var_indices_binary);
        sensitive << a << as << as_i << s;

    }

//-----------------------------------------------------------------------------
    // Function with records
    struct Simple {
        int a;
        sc_uint<2> b;
    };
    
    Simple rec[16];

    struct Rec {
        unsigned int a;
        sc_uint<16> b;
    };

    Rec rec2[6];
    Rec rec3[12];


    void record_var_indices()
    {
        int num = 2;

        rec[0].a = 10;
        rec[1].a = 15;
        rec[num].a = 20;

        rec[a.read()].a = 25;
        rec[as.read()].a = 30;
        rec[as_i.read()].a=35;
        rec[s.read()].a=40;


        bool flag;
    }
    void record_var_indices_arith()
    {
        int num = 2;

        rec3[0].b = 10;
        rec3[1].b = rec3[0].b + 2;

        rec3[num].b = (rec3[num-2].b - rec3[num-1].b);
        rec3[num+1].b = (rec3[num-2].b * rec3[num-1].b);
        rec3[num+2].b = (rec3[num-2].b / rec3[num-1].b);


        sct_assert_const(rec3[num-2].b==10);
        sct_assert_const(rec3[num-1].b==12);
        sct_assert_const(rec3[num].b==65534);
    }

    void record_var_indices_binary()
    {
        int num = 2;

        rec2[0].b = 10;
        rec2[1].b = rec2[0].b & 2;

        rec2[num].b = ~(rec2[num-2].b | rec2[num-1].b);

        sct_assert_const(rec2[num-2].b==10);
        sct_assert_const(rec2[num-1].b==2);
        sct_assert_const(rec2[num].b==65525);
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

