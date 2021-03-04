/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// for statement with function call in condition and initialization not supported,
// error reported
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<bool>     s{"s"};
    
    int                 m;
    int                 k;
    
    SC_CTOR(A) {
        SC_METHOD(func_call_cond1);
        sensitive << a;

        //SC_METHOD(func_call_cond2);
        //sensitive << s;
    }
    
    // For with function call in condition w/o comparison
    bool cond(int i) {
        return (i < 2);
    }
    
    void func_call_cond1() 
    {
        int i = 0;
        while (cond(i)) {
            i++;
        }
    }
    
    
    void func_call_cond2() 
    {
        int i = s.read();
        do {
            i++;
        } while (cond(i));
        i = 1;
    }
};

class B_top : public sc_module 
{
public:

    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
    }
};

int  sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

