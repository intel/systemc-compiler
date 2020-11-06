/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 12/4/18.
//

#include <systemc.h>

SC_MODULE(dut) {
    sc_in<int> in{"in"};
    SC_CTOR(dut) {}
};

SC_MODULE(test) {
    dut *mod_arr[3];
    sc_signal<int> signals[2];
    SC_CTOR(test) {
        mod_arr[0] = new dut("dut0");
        mod_arr[2] = new dut("dut2");

        mod_arr[0]->in(signals[0]);
        mod_arr[2]->in(signals[1]);
    }
};


int sc_main(int argc, char **argv) {
    test t{"t"};
    sc_start();
    return 0;
}
