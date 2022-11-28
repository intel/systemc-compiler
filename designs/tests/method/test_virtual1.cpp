/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Virtual function call in the module class with overloaded functions

struct A : public sc_module
{
    unsigned int                 m;
    sc_signal<bool> dummy{"dummy"};

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(virt_call); sensitive << dummy;
    }
    
    virtual void d() {
        m = 1;
    }
    virtual int d(sc_uint<3> val) {
        return (val+1);
    }
    virtual void f() {
        m = 0;
    }
    virtual void f(int i) {
        m = i;
    }

    void virt_call() {
        sc_uint<3> u = 3;
        d(u + 1);
        f();
        f(5);
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
        m = -2;
    }
    virtual int d(sc_uint<3> val) {
        m = -val;
        return (val-1);
    }
    virtual void f() {
        m = -1;
    }
    virtual void f(int i) {
        m = -i;
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
