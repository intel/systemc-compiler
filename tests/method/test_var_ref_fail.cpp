/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// ...
class A : public sc_module {
public:
    sc_signal<bool> s;


    SC_CTOR(A) {
        SC_METHOD(reference_inc_decr); sensitive << s;
    }

    
    int f (int& i) {
        return 42;
    }
    
    void reference_inc_decr() 
    {
        int a;
        int m = 1;
        // Double increment
        int& r = ++m;
        a = r + r;
        
        // Decrement not used in function
        f(--m);
    }
    
    
};

int sc_main(int argc, char *argv[]) {
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

