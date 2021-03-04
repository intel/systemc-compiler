/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Virtual function call with access base class member variable/channel

struct A : public sc_module
{
    sc_signal<bool>     s;
    int                 m;

    sc_signal<bool> dummy{"dummy"};

    SC_HAS_PROCESS(A);
    
    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(virt_call); sensitive << dummy;
    }
    
    virtual void f() {
        int m = 1;
    }

    void virt_call() {
        f();
    }
};


// Method process body analysis
class C : public A 
{
public:
    C(const sc_module_name& name) : A(name) 
    {}
    
    virtual void f() {
        short m = 2;
        s = 1;
    }
};

// Method process body analysis
class D : public C 
{
public:
    SC_HAS_PROCESS(D);

    sc_signal<bool> dummy{"dummy"};

    D(const sc_module_name& name) : C(name) {
        SC_METHOD(virt_call_d); sensitive << dummy;
    }
    
    virtual void f() {
        char m = 3;
        s = 0;
    }

    void virt_call_d() {
        f();
    }
};


class B_top : public sc_module 
{
public:
    C   c_mod{"c_mod"};
    D   d_mod{"d_mod"};

    B_top(const sc_module_name& name) : sc_module(name) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
