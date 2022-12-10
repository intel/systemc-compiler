/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/8/18.
//

#include <systemc.h>

class top : sc_module {
public:

    sc_signal<int> *sig_ptr = new sc_signal<int> ("sig_ptr");

    sc_signal<int> *sig_parray[2];

    const int *int_array = new int[2]{22,33};


    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        sig_parray[0] = new sc_signal<int>("sig_parray0");
        sig_parray[1] = new sc_signal<int>("sig_parray1");

        SC_METHOD(test_method);
        sensitive << *sig_ptr << *sig_parray[1];
    }

    void test_method() {
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_dynalloc\n";
    top t{"t"};
    sc_start();
    return 0;
}

