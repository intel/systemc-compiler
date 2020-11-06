/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 4/10/18.
//

#include <systemc.h>

struct foo {
    static const int cfoo = 12;
    const unsigned cfoo2 = 13;
};

struct base {
    static const int cbase0 = 20;
    static const uint16_t cbase1 = 21;
    const unsigned cbase2 = 22;
};

struct top : sc_module, base {

    unsigned x0;
    const int x1 = 1;
    const char x2 = 2;
    uint64_t x3;
    int64_t  x4;
    const int cx = 1;
    const sc_int<10> csi = 10;
    const sc_uint<10> csui = 11;
    sc_biguint<512> buint;
    sc_bigint<128> bint;

    int array[2][2];

    static const int static_c0 = 101;
    static const uint64 static_c1 = 102;
    static constexpr int static_c2 = 103;

    static constexpr int static_carray[] = { 1, 2, 3, 4};
    static constexpr uint16_t static_carray2d[][2] = { {1,2}, {3,4} };


    sc_signal<int> sig_int{"sig_int"};
    sc_signal<unsigned > sig_uint{"sig_uint"};

    foo f;

    SC_CTOR(top) {
//        SC_METHOD(test_method);
    }

    void test_method() {}

};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

