/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

void glob_f() {}
bool glob_g() {return 1;}

// Method with empty sensitivity with function/method call
class A : public sc_module {
public:
    sc_in<bool>     a{"a"};
    sc_out<bool>    b{"b"};

    SC_CTOR(A) {
        SC_METHOD(empty_call1); 
        SC_METHOD(empty_call2); 
    }
    
    void f() {}
    int g() {return 42;}
    
    void empty_call1() 
    {
        f();
        int i = g();
    }
    
    void empty_call2() 
    {
        if (glob_g()) {
            glob_f();
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

