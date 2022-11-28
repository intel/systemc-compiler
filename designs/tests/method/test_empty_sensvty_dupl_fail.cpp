/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Method with empty sensitivity list with duplicated initialization failed
class A : public sc_module {
public:
    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;

    sc_signal<bool> s1;
    sc_signal<int>  s2;

    SC_CTOR(A) {
        SC_METHOD(empty);
    }
    
    void empty() 
    {
        s1 = 0;
        if (!CONST_Z) {
            s1 = 1;
        }
        int i = 1;
        if (CONST_A) {
            i = 2;
            s2 = i+1;
        }
    }

};

int sc_main(int argc, char *argv[]) {
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

