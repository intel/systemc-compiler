/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 12/7/18.
//

#include <systemc.h>

class top : sc_module {
public:

    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        SC_METHOD(test_method);
        sensitive << dummy;
    }

    sc_signal<bool> dummy{"dummy"};

    int array2d[2][2];
    int array3d[2][2][2];

    void test_method() {
        array2d[1][1] = array3d[0][1][1];
    }

};


int sc_main(int argc, char* argv[])
{
    top t{"t"};
    sc_start();
    return 0;
}

