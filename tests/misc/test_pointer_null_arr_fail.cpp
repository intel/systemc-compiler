/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

using namespace sc_core;

// Null/non-initialized array of pointers access error detection
class A : public sc_module {
public:
    bool*               arr[3];
    sc_signal<bool>*    sarr[3];
    
    sc_signal<int>         dummy;
    
    SC_CTOR(A) {
        for (int i = 0; i < 3; i++) {
            //arr[i] = nullptr; //sc_new<bool>();
            //sarr[i] = nullptr;
        }
        SC_METHOD(meth); 
        sensitive << dummy;
    }
    
    void meth() 
    {
        int i = dummy.read();
        *arr[i] = 0;
        //*sarr[i] = 0;
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


