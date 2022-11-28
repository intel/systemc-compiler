/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Method with empty sensitivity list with if/? and local variables
class A : public sc_module {
public:
    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;
    
    sc_in<bool>     a{"a"};
    sc_out<bool>    b{"b"};
    sc_out<bool>    c{"c"};
    sc_out<int>     d{"d"};

    sc_signal<int>  s1{"s1"};
    sc_signal<int>  s2{"s2"};
    sc_signal<int>  s3{"s3"};
    sc_signal<int>  s4{"s4"};
    sc_signal<sc_uint<4>>  s5{"s5"};
    sc_signal<int>  s6{"s6"};
    sc_signal<int>  s7{"s7"};
    sc_signal<sc_uint<4>> s8;

    SC_CTOR(A) {
        SC_METHOD(empty_decl);

        SC_METHOD(empty_cond);
        
        SC_METHOD(empty_if1);
        SC_METHOD(empty_if2);
    }
    
    void empty_decl() 
    {
        int i = 0;
        const sc_uint<4> j = 1;
        s5 = j-1;
        s6 = i+1;
    }
    
    void empty_cond() 
    {
        sc_uint<4> i = 1;
        sc_uint<4> j = 2;
        s7 = CONST_A ? 1 : CONST_Z;
        s8 = (!CONST_Z) ? i : j;
    }
    
    void empty_if1() 
    {
        if (CONST_Z) {
            b = 1;
        } else {
            s1 = 1;
        }
        if (CONST_A) {
            c = false;
        } else {
            s1 = 2;
        }
        s2 = 3;
        
        sc_int<8> i = 1;
    }
    
    void empty_if2() 
    {
        if (!CONST_Z) {
            b = 1;
        }
        if (CONST_A) {
            d = 2;
            if (!CONST_A) {
            } else 
                s3 = 1;
            s4 = 2;
        }
    }

};

class B_top : public sc_module {
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};
    sc_signal<int>  e{"e"};
    sc_signal<int>  d{"d"};
    sc_signal<int>  p1{"p1"};
    sc_signal<int>  p2{"p2"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
        a_mod.d(d);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

