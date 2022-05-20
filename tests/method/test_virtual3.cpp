/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Virtual function call in the module class with different class parameter access

struct A : public sc_module
{
    char               m;

    SC_HAS_PROCESS(A);

    sc_signal<bool> dummy{"dummy"};

    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(virt_call); sensitive << dummy;
    }
    
    virtual void f() {
        m = 1;
    }

    void virt_call() {
        f();
    }
};


// Method process body analysis
class C : public A 
{
public:
    short               m;

    C(const sc_module_name& name) : A(name) 
    {}
    
    virtual void f() {
        m = 2;
    }
};

// Method process body analysis
class D : public C 
{
public:
    int                 m;

    D(const sc_module_name& name) : C(name) 
    {}
    
    virtual void f() {
        m = 3;
        C::m = 3;
        A::m = 3;
        int i = m + C::m + A::m;
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
