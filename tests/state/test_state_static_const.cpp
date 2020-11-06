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

    static const unsigned const_unsigned0 = 1 + 2 + 3;
    static const unsigned const_unsigned1 = 1 + 2 + 3 + const_unsigned0;

    constexpr static const int const_array [2] = {1+1, 2+2};

    constexpr static const int const_array2d [2][2] = {40 + 1 + const_unsigned0
                                                     , 40+2, 40+3, 40+4};

    constexpr static const unsigned const_array3d[1][1][1] = {111};

    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        SC_METHOD(test_method);
    }

    void test_method() {
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_static_const\n";
    top t{"t"};
    sc_start();
    return 0;
}

