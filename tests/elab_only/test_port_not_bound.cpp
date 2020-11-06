/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 02/08/18.
//

#include <systemc.h>

// Not connected port -- test will FAIL
struct top : sc_module {
    sc_signal<int> sig{"sig"};
    
    // Various kinds of not-connected port to check field name is reported
    sc_in<int> in;
    sc_in<int> in_[10];
    sc_in<int> in__[10][3];

    sc_in<int>* in_p;
    sc_in<int>* in_arr[2];
    sc_in<int>* in_arr2[2][2];

    SC_CTOR(top) 
    {
        in_p = new sc_in<int>("in_name");
        for (int i = 0; i < 2; i++) {
            in_arr[i] = new sc_in<int>("in_arr_name");
            for (int j = 0; j < 2; j++) {
                in_arr2[i][j] = new sc_in<int>("in_arr2_name");
            }
        }
    }
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports\n";
    top t0("t0");
    
    sc_start();
    return 0;
}

