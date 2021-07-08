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
    sc_signal<sc_uint<4>>  d{"d"};
    sc_signal<int>  s;

    SC_CTOR(A) 
    {
        SC_METHOD(latch_fail1); sensitive << a;
        SC_METHOD(latch_fail2); sensitive << a << b;
        SC_METHOD(latch_fail3); sensitive << b;
        SC_METHOD(latch_fail4); sensitive << a << b;
        
        SC_METHOD(latch_switch_fail); sensitive << c << s;
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
            d = 42;
        }
    }
    
    sc_signal<sc_uint<1>> w;
    void latch_fail4() 
    {
        if (a) {
            for (int i = 0; i < 3; ++i) {
                if (b) {
                    w = 42;
                }
            }
            w = 43;
        }
    }

    sc_signal<sc_uint<1>> r;
    sc_signal<sc_uint<1>> t;
    void latch_switch_fail() 
    {
        switch (c.read()) {
            case 0: break;
            case 1: 
                if (s.read()) {
                    r = 1;
                } else {
                    t = 2;
                }
                break;
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

