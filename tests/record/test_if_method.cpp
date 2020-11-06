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

// Record simple tests
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) 
    {
        SC_METHOD(record_fcall);
        sensitive << dummy;

        SC_METHOD(record_fcall2);
        sensitive << dummy;
    }

    struct Simple {
        bool a;
        int b;
    };
    
    void seta(Simple& par){
        par.a = true;
    }

    void seta(Simple& par, bool val){
        par.a = val;
    }

    bool f(Simple par) {
        return par.a;
    }

    void record_fcall()
    {
        Simple s1; 
        bool c = f(s1);
        if (c) {
            Simple s2;
            s2.b = 42;
        }
    }
    
    Simple rec;
    void record_fcall2()
    {
        rec.a = dummy.read();              
        Simple s1; 
        seta(s1);

        Simple s2;
        seta(s2);

        bool flag = f(s1);

        if (rec.a) {
            int s1 = s2.b;
        }
    }
};

class B_top : public sc_module {
public:
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

