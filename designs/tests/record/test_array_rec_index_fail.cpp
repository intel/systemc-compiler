/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Record array elements accessed at the same record index -- error reported, #99
struct MyRec {
    sc_int<2>  a;
    sc_uint<4> b;
};
    
class A : public sc_module {
public:
    sc_signal<int>        s{"s"};

    SC_CTOR(A) {
        SC_METHOD(rec_indx0);
        sensitive << s;
        
        SC_METHOD(rec_indx1);
        sensitive << s;
    }

    sc_signal<int> t0;
    void rec_indx0()
    {
        int a[3];
        a[a[1]] = 1;                    // OK
        
        MyRec err[3];
        err[err[1].b].a = 1;            // Error
        t0 = err[err[s.read()].b].a;    // Error
        err[1+err[s.read()].a].a = 1;   // Error

        MyRec idx[3];
        err[idx[1].b].a = 1;            // OK
        
        t0 = err[0].a;
    }
    
    MyRec mb[3];
    MyRec mi[3];
    sc_signal<int> t1;
    void rec_indx1()
    {
        MyRec mb[3];
        mb[mb[1].b].a = 1;              // Error
        t1 = mb[mi[mb[1].a].b].a + 1;   // Error
        
        mb[mi[s.read()].b].a = 1;       // OK
        t1 = mb[0].a + mi[mb[1].b].a;   // OK
        
        t1 = mb[0].a;
    }
};

int sc_main(int argc, char *argv[]) {
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

