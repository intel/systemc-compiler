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


SC_MODULE(deep_bottom) {

    sc_in <int> in{"in"};
    sc_out <int> out{"out"};

    SC_CTOR(deep_bottom) { }
};


SC_MODULE(bottom) {

    sc_in <int> in{"in"};
    sc_out <int> out{"out"};
    deep_bottom db{"db"};

    SC_CTOR(bottom) {}
};

SC_MODULE(top) {

    sc_signal<int>  sig0{"sig0"};
    sc_signal<int>  sig1{"sig1"};

    bottom b{"b"};

    SC_CTOR(top) {
        b.in(sig0);
        b.out(sig0);
        b.db.in(sig1);
        b.db.out(sig1);
    }
};

int sc_main (int argc, char **argv) {
    top t0{"t0"};
    sc_start();
    return 0;
}

