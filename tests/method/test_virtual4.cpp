/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Virtual function call in different class instances

struct E : public sc_module, public sc_interface {
    int n;
    
    E(const sc_module_name& name) : sc_module(name) {
    }
    
    void g() {
        n = 0;
    }
    void g(int i) {
        n = i;
    }
};

struct A : public sc_module
{
    int                 m;
    
    E   e1{"e1"};
    E   e2{"e2"};

    SC_HAS_PROCESS(A);

    sc_signal<bool> dummy{"dummy"};

    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(virt_call); sensitive << dummy;
    }
    
    virtual void d() {
        e2.g(1);
    }

    void virt_call() {
        e1.g();
        d();
    }
};


// Method process body analysis
class C : public A 
{
public:
    int                 m;
    
    C(const sc_module_name& name) : A(name) 
    {}
    
    virtual void d() {
        e2.g(2);
    }
};


class B_top : public sc_module 
{
public:
    C c_mod{"c_mod"};

    B_top(const sc_module_name& name) : sc_module(name) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
