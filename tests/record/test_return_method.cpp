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

// Record (structure/class) non-module tests
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(record_return1);
        sensitive << dummy;

        SC_METHOD(record_return2);
        sensitive << dummy;

        SC_METHOD(record_return3);
        sensitive << dummy;
    }
    
//-----------------------------------------------------------------------------
    // Function with records
    struct Simple {
        bool a;
        int b;
    };
    
//-----------------------------------------------------------------------------
    // Function with record in return
    Simple f() {
       Simple r;
       r.b = 2;
       return r;
    }
     
    void record_return1() 
    {
        Simple s = f();
        int i = s.b + 1;
    }

    Simple g(bool val1, int val2) {
       Simple r;
       r.a = val1;
       r.b = val2;
       return r;
    }
    
    void record_return2() 
    {
        Simple s = g(true, 2);
        if (s.a) {
            s.b = 1;
        }
    }
    
    void record_return3() 
    {
        Simple s;
        if (dummy) {
            s = f();
        }
        int i = s.b;
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

