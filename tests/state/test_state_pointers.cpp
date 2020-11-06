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

    sc_signal<int> *sp0;
    sc_signal<int> sigint{"sigint"};
    sc_signal<int> *sp1 = &sigint;


    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        sp0 = &sigint;
        SC_METHOD(test_method);
    }

    void test_method() {
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_pointers\n";
    top t{"t"};
    sc_start();
    return 0;
}

