/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// C++ @sizeof()
SC_MODULE(A) {
    sc_signal<int> s;
    
    SC_CTOR(A) {
        SC_METHOD(methodProc);
        sensitive << s;
    }    
    
    struct Rec {
        int a;
        bool b;
    };
    
    sc_signal<int> t;
    void methodProc() {
    	unsigned i;
        i  = sizeof(unsigned);
        sct_assert_const(i == 4);
        printf("U %d\n", i);
    	i = sizeof(long unsigned);
        sct_assert_const(i == 8);
        printf("LU %d\n", i);
        i = sizeof(long unsigned);
        sct_assert_const(i == 8);
        printf("LLU %d\n", i);
        i = sizeof(unsigned*);
        sct_assert_const(i == 8);
        printf("ptr %d\n", i);
        i = sizeof(float);
        printf("ptr %d\n", i);
        i = sizeof(Rec);
        printf("Rec %d\n", i);
        i = sizeof(sc_uint<16>);
        printf("sc_uint<16> %d\n", i);
        i = sizeof(sc_biguint<64>);
        printf("sc_biguint<64> %d\n", i);
        t = i;
    }
};


int sc_main(int argc, char **argv) {

    cout << "test_sizeof\n";

    A a_mod{"a_mod"};
    sc_start();

    return 0;
}


