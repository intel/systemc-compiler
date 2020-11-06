/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

using namespace sc_core;

// Multiple pointers to variable (non-module), no error/warning reported
class A : public sc_module {
public:
    bool        a;
    bool*       p;
    bool*       q;

    sc_signal<int>          s;
    sc_signal<int>*         ps;
    sc_signal<int>*         qs;

    sc_signal<bool>         dummy;
    
    SC_CTOR(A) 
    {
        p = &a; 
        q = &a;
        
        ps = &s;
        qs = &s;
        
        SC_METHOD(meth); 
        sensitive << *qs << *ps;
    }
    
    void meth() {
        *q = qs->read();
        *p = ps->read() + 1;
    }     
    
};

class B_top : public sc_module {
public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}


