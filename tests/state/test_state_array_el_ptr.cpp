/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/9/18.
//

#include <systemc.h>

class top : sc_module {
public:

    const int *carray_el_ptr;
    int (*nonconst_array_ptr)[2][2];
    sc_signal<int> * sig_ptr;
    sc_signal<int> (*sig_array_ptr)[3];

    const int const_array [2][2] = {1, 2, 3, 4};
    int nonconst_array [2][2] = {44, 44, 44, 44};
    sc_signal<int> sig_array[3];

//    int * non_const_el_ptr = &nonconst_array[1][1];

    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        SC_METHOD(test_method);
        sensitive << sig_array[1];
        carray_el_ptr = &const_array[1][1];
        nonconst_array_ptr = &nonconst_array;

        sig_ptr = &sig_array[1];
        sig_array_ptr = &sig_array;
    }

    void test_method() {
        sig_array[2] = 122;
        (*nonconst_array_ptr)[0][1] = 12;
        nonconst_array[1][1] = 1;
        (*sig_array_ptr)[0].write( 1 );
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_array_el_ptr\n";
    top t{"t"};
    sc_start();
    return 0;
}

