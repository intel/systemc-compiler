/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// SWITCH statement with empty cases only -- error reported
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    
    int                 m;

    sc_signal<bool>         dummy;
    
    SC_CTOR(A) {
        
        SC_METHOD(switch_empty1); sensitive << dummy;
        SC_METHOD(switch_empty2); sensitive << dummy;
    }
    
    void switch_empty1() {
        int i;
        switch (m) {
            default: ;
        }
        i = 0;
    }
    
    void switch_empty2() {
        int i;
        switch (m) {
            case 1: ;
            default: ;
        }
        i = 0;
    }
    
};

struct dut : sc_core::sc_module {
    typedef dut SC_CURRENT_USER_MODULE;
    dut(::sc_core::sc_module_name) {}
};

class B_top : public sc_module
{
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

