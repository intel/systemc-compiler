/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

using namespace sc_core;

// Null and non-initialized pointer access error detection
class A : public sc_module {
public:
    sc_uint<4>*             x;
    sc_signal<bool>         dummy;
    
    SC_CTOR(A) {
        x = nullptr;
        SC_METHOD(meth); 
        sensitive << dummy;
    }
    
    void meth() 
    {
        x->bit(0) = 1;
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


