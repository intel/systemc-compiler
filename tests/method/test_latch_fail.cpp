/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Method with latch, these test must fail
class A : public sc_module {
public:
    sc_in<bool>     a{"a"};
    sc_out<bool>    b{"b"};
    sc_signal<sc_uint<4>>  c{"c"};

    SC_CTOR(A) {
        SC_METHOD(latch_fail1); 
        sensitive << a;

        SC_METHOD(latch_fail2); 
        sensitive << a << b;

        SC_METHOD(latch_fail3); 
        sensitive << b;
    }
    
    void latch_fail1() 
    {
        if (a) {
            b = 1;
        }
    }
    
    void latch_fail2() 
    {
        if (a || b) {
            c = (a.read()) ? 1 : 2;
        }
    }
    
    void latch_fail3() 
    {
        if (false || b) {
            c = 42;
        }
    }
};

class B_top : public sc_module {
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

