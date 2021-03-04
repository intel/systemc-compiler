/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constant static arrays
static const sc_int<5> const_array[] = {1,2,3,4};
static const sc_int<5> const_array2d[2][2] = {{1,2},{3,4}};

SC_MODULE(top) {

    //static const sc_int<5> const_array3d[2][2][1];

    static const sc_int<5> const_val;

    sc_signal<bool> dummy {"dummy"};

    sc_signal<sc_int<5>> out0{"out0"};
    sc_signal<sc_int<5>> out1{"out1"};
    sc_signal<sc_int<5>> out2{"out2"};


    SC_CTOR(top) {
        SC_METHOD(test_method);
        sensitive << dummy;

    }

    void test_method() {

        out0 = const_array[1];
        out1 = const_array2d[1][0];
        out2 = const_array3d[1][1][0];
        out2 = const_val;

        cout << const_array[1] << endl;
        cout << const_array2d[1][0] << endl;
        cout << const_array3d[1][1][0] << endl;
    }

};

//const sc_int<5> top::const_array3d[2][2][1] = { {{1}, {2}},   {{3}, {4}} };
const sc_int<5> top::const_val = -1;

int sc_main(int argc, char **argv) {
    top t0{"t0"};
    sc_start();
    return 0;
}
