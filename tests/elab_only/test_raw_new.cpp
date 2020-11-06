/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/14/18.
//

#include <systemc.h>

SC_MODULE(top) {

    const int SIZE = 2;

    SC_CTOR(top) {
        ival_p = sc_new<int>();
        short_array_p = sc_new_array<short>(SIZE);
//        const_short_array_p = new short[SIZE]{42, 43};

        signal_array_p = sc_new_array<sc_signal<bool> *>(SIZE);
        for (size_t i = 0; i < SIZE; ++i) {
            signal_array_p[i] = new
                sc_signal<bool>(sc_gen_unique_name("signal_array"),
                                static_cast<bool>(i % 2));
        }
    }

    int *ival_p;
    short *short_array_p;
//    const short *const_short_array_p;
    sc_signal<bool> **signal_array_p;

    sc_signal<int> *signal_array2_p;
};

int sc_main(int argc, char **argv) {
    top t{"t"};
    sc_start();
    return 0;
}
