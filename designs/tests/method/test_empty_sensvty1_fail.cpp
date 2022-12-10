/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Method with empty sensitivity list read signal fail
class A : public sc_module {
public:
    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;
    
    sc_in<bool>     a{"a"};
    sc_out<bool>    b{"b"};
    sc_out<bool>    c{"c"};
    sc_in<int>      d{"d"};

    sc_signal<int>  s1{"s1"};
    sc_signal<int>  s2{"s2"};

    SC_CTOR(A) {
        SC_METHOD(empty_decl);
        SC_METHOD(empty_decl2);
        SC_METHOD(empty_cond);
    }
    
    void empty_decl() 
    {
        int i;
        i = a.read();       // Error reported
        b = i + 1;
    }
    
    void empty_decl2() 
    {
        int i = a.read();   // Error reported
        sc_uint<3> j = i -1;
        b = j + 1;
    }

    void empty_cond() 
    {
        s2 = (b) ? a.read() : 0;
        sc_uint<3> x = (a) ? d.read()+1 : d.read()-1;
        b = (x == s1.read()) ? 1 : 0;  // Error reported
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

