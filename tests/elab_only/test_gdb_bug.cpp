/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/26/18.
//

#include <systemc.h>

SC_MODULE(top) {
    // we removed implicit instantiations for sc_in<bool> from systemc kernel
    // without hacks in systemc kernel gdb reports sizeof(in) == 0
    sc_in<bool> in[1][1];
    sc_signal<bool> sig[1][1];

    SC_CTOR(top) {
        in[0][0].bind(sig[0][0]);
    }

};

int sc_main (int argc, char **argv) {
    cout << "test_gdb_bug\n";
    top t0{"t0"};
    sc_start();
    return 0;
}

