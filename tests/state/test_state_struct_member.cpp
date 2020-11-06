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

struct complex {
    int re = 10101;
    int im = 10101;
};

class top : sc_module {
public:

    complex cmplx {1,2};
    complex cmparray[2];

    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        SC_METHOD(test_method);
    }

    void test_method() {
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_struct_member\n";
    top t{"t"};
    sc_start();
    return 0;
}

