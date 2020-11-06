/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 11/25/18.
//

#include <systemc.h>

static int sx = 0;

struct MyPair {
    int x;
    int y;
};

struct PairPair {
    MyPair a;
    MyPair b;
};

SC_MODULE(test) {

    sc_signal<bool> dummy;

    SC_CTOR(test) {
        SC_METHOD(test_method);
        sensitive << dummy;

        // Not supported for generation yet
        //SC_METHOD(tuple_method);
        //sensitive << dummy;
    }

    std::pair<int, sc_int<10>> intPair;

    std::pair<int, sc_int<10>> intPairArray[2];

    std::tuple<int, int, int> intTuple;

    std::array<int, 10> intArray;

    MyPair mpa[2];

    PairPair ppa[2];

    void test_method() {
        intPair.first = 12;
        intPair.second = 13;


        // TODO : FIXME
        intPairArray[1].second = 12;

        mpa[0].y = mpa[0].x;
        mpa[1].y = mpa[1].x;

        ppa[0].a.x = 1;
        ppa[1].b.y = 0;
        int idx = dummy;
        // intPairArray[idx].first = 1;

        // TODO:
        // intArray[0] = 0;

        int idx1 = dummy;
        int idx2 = dummy;

//        mpa[idx1].x = 1;
    }
    
    void tuple_method() {
        // Not supported for generation yet
        std::get<0>(intTuple) = 12;
    }

};

int sc_main(int argc, char **argv) {
    test t{"t"};
    sc_start();

    return 0;
}
