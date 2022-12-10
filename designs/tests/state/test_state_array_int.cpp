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

    const int const_array [2][2] = {1, 2, 3, 4};
    int nonconst_array [2][2] = {44, 44, 44, 44};

    const sc_int<12> scint_const_array [2][2] = {10, 11, 12, 13};

    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        SC_METHOD(test_method);
    }

    void test_method() {
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_array_int\n";
    top t{"t"};
    sc_start();
    return 0;
}

