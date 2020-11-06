/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 1/29/18.
//

#include <systemc.h>

SC_MODULE(mod_with_param) {
    sc_vector<sc_in<bool>> din;

    mod_with_param(::sc_core::sc_module_name, size_t din_size)
    : din("din", din_size)
    {}
};

SC_MODULE(top) {
    mod_with_param m0{"m0",1};
    mod_with_param m1{"m1",2};

    sc_vector<sc_signal<bool>> m0_din{"m0_din",1};
    sc_vector<sc_signal<bool>> m1_din{"m1_din",2};

    SC_CTOR(top) {
        m0.din(m0_din);
        m1.din(m1_din);
    }
};

int sc_main (int argc, char **argv) {
    top t0{"t0"};
    sc_start();
    return 0;
}
