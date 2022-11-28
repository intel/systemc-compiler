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

// Record (structure/class) non-module field use in assert/sct_assert
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) 
    {
        SC_METHOD(record_assert);
        sensitive << dummy;

        SC_METHOD(record_sct_assert);
        sensitive << dummy;

        SC_METHOD(record_sct_assert_fcall);
        sensitive << dummy;
    }
    
    // Function with records
    struct Simple {
        bool a;
        int b;
    };
    
    void record_assert() 
    {
        Simple s;
        s.b = 1;
        assert(s.b == 1);
    }

    void record_sct_assert() 
    {
        Simple s;
        s.b = 2;
        sct_assert_const(s.b == 2);
    }

    Simple f(bool val) {
        Simple r;
        r.a = val;
        r.b = val ? 2 : 3;
        return r;
    }
    
    void record_sct_assert_fcall() 
    {
        Simple s = f(false);
        sct_assert_const(!s.a);
        sct_assert_const(s.b == 3);
    }

    bool g() {
        return false;
    }

    void return_bool() 
    {
        bool a = g();
        sct_assert_const(!a);
    }
    
};
    
class B_top : public sc_module {
public:
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

