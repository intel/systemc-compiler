/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 6/16/18.
//

#include <systemc.h>

SC_MODULE(a_mod) {
    sc_signal <int> a_sig{"a_sig"};
    SC_CTOR(a_mod) {}
};

SC_MODULE(b_mod) {
    sc_signal <bool> b_sig{"b_sig"};
    SC_CTOR(b_mod) {}
};

SC_MODULE(top) {

    std::unique_ptr<sc_module> modp;

    SC_CTOR(top) {
    }

    void test_method() {}
};

int sc_main (int argc, char **argv) {
    cout << "test_unique_ptr\n";
    top t0{"t0"};
    sc_start();
    return 0;
}

