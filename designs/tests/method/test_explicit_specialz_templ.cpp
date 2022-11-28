/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"
#include <iostream>

template <int N>
SC_MODULE (A) {
public:
    sc_signal<bool> dummy;
    sc_in<int> a;
    sc_out<int> c;
    void add() { c = a + N;}


     SC_CTOR(A) : a("a"), c("c") {
        SC_METHOD(add);
        sensitive << a;
     }
};

template<> SC_MODULE(A<7>)        { // EXPLICIT SPECIALIZATION
    sc_in<int> a;
    sc_out<int> c;
    void add() { c = a + 3; }
    SC_CTOR(A) : a("a"), c("c") {
        SC_METHOD(add);
        sensitive << a;
    }
};

class B_top : public sc_module
{
public:
    sc_signal<int>  a{"a"};
    sc_signal<int>  c{"c"};
    sc_signal<int>  ya{"ya"};
    sc_signal<int>  yc{"yc"};

    A<7> x{"x"};
    A<4> y{"y"};

    SC_CTOR(B_top) {
        x.a(a);
        x.c(c);
        y.a(ya);
        y.c(yc);

    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

