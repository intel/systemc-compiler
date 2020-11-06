/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/24/18.
//

#include <systemc.h>

SC_MODULE(top) {

    sc_in<int> in0{"in0"};
    sc_in<int> in1{"in1"};
    sc_signal<int> sig{"sig"};
    sc_out<int> out{"out"};


    SC_CTOR(top) {
        in0.bind(in1);
        in1.bind(sig);
        out.bind(sig);
    }
};

int sc_main (int argc, char **argv) {
    top t0{"t0"};
    sc_start();
    return 0;
}

