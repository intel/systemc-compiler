/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Child module port access -- that is error
struct A : public sc_module
{
    sc_in<bool>     in{"in"};
    sc_out<bool>    out{"out"};
    bool            var;

    A(const sc_module_name& name) : sc_module(name) {
    }
};


class C : public sc_module
{
public:
    A a;
    sc_signal<bool> s{"s"};
    
    SC_HAS_PROCESS(C);
    
    C(const sc_module_name& name) : 
        sc_module(name),
        a("a") 
    {
        SC_METHOD(proc);
        sensitive << s;
    }
    
    void proc() {
        bool b;
        // Any of these accesses leads to fatal error
        //b = a.in.read();
        //b = a.var;
        a.out = b;
    }
};


class B_top : public sc_module 
{
public:
    C c_mod{"c_mod"};
    sc_signal<bool> s1{"s1"};

    B_top(const sc_module_name& name) : sc_module(name) {
        c_mod.a.in(s1);
        c_mod.a.out(s1);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
