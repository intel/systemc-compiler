/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constant static arrays
static const sc_int<5> const_array[] = {1,2,3,4};
//static const sc_int<5> const_array2d[2][2] = {{1,2},{3,4}};

SC_MODULE(top) {
    static constexpr unsigned const_array_i[4] = {2,3,4,5};
    static constexpr unsigned const_array_j[2][2] = {{2,3},{4,5}};
    
    // Uncomment me after #223
    //static const sc_int<5> const_array_m[];
    //static const sc_int<5> const_array3d[2][2][1];

    static const sc_int<5> const_val;

    sc_signal<bool> s {"s"};

    sc_signal<sc_int<5>> out0{"out0"};
    sc_signal<sc_int<5>> out1{"out1"};
    sc_signal<sc_int<5>> out2{"out2"};


    SC_CTOR(top) {
        SC_METHOD(test_method);
        sensitive << s;

    }

    void test_method() {

        out0 = const_array[1];
        out0 = const_array[s.read()];
        //out1 = const_array_m[2];
        //out1 = const_array_m[s.read()];
        out2 = const_array_i[2];
        out2 = const_array_i[s.read()];
        out2 = const_array_j[1][0];
        out2 = const_array_j[s.read()][1];

        //out1 = const_array2d[1][0];
        //out2 = const_array3d[1][1][0];

        cout << const_array[1] << endl;
        cout << const_array[2] << endl;
        //cout << const_array2d[1][0] << endl;
        //cout << const_array3d[1][1][0] << endl;
    }

};

//const sc_int<5> top::const_array_m[] = {2,3,4,5};
//const sc_int<5> top::const_array3d[2][2][1] = { {{1}, {2}},   {{3}, {4}} };
const sc_int<5> top::const_val = -1;

int sc_main(int argc, char **argv) {
    top t0{"t0"};
    sc_start();
    return 0;
}
