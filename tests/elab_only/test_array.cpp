/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 12/15/17.
//

#include <systemc.h>
#include <typeinfo>

struct top : sc_module {

    SC_CTOR(top) {
        cout << *cp1 << endl;
        cout << *p2 << endl;
        cout << p2_2[0][0] << endl;
        cout << p2_22[0][0][0] << endl;
        cout << *cp3 << endl;
        cout << cp3_2[0][0] << endl;

        sc_object *obj = &signal_array[0];

        cout << typeid(*obj).name();
    }

    const int cint_array_1d[2] = {11, 12};
    int       int_array_2d[2][2] = {{21, 22}, {23, 24}};
    const int cint_array_3d[2][2][2] = {{{31, 32}, {33, 34}}, {{35, 36}, {37, 38}}};

    const int *cp1 = &cint_array_1d[1];
    int *p2 = &int_array_2d[1][1];
    int (*p2_2)[2]  = &int_array_2d[1];
    int (*p2_22)[2][2]  = &int_array_2d;
    const int *cp3 = &cint_array_3d[1][0][0];
    const int (*cp3_2)[2]  = &cint_array_3d[1][1];

    sc_signal<int> signal_array[3];

};

int sc_main(int argc, char** argv)
{
    top top_inst{"top_inst"};
    sc_start();
    return 0;
}

