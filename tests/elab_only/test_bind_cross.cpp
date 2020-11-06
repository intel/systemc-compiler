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

SC_MODULE(deep_bottom0) {
    sc_in <int> in{"in"};
    sc_out <int> out{"out"};
    sc_signal <int> sig{"sig"};
    SC_CTOR(deep_bottom0) { }
};

SC_MODULE(bottom0) {
    sc_in <int> in{"in"};
    sc_out <int> out{"out"};
    sc_signal <int> sig{"sig"};
    deep_bottom0 db0{"db0"};
    SC_CTOR(bottom0) {}
};

SC_MODULE(deep_bottom1) {
    sc_in <int> in{"in"};
    sc_out <int> out{"out"};
    sc_signal <int> sig{"sig"};
    SC_CTOR(deep_bottom1) { }
};

SC_MODULE(bottom1) {
    sc_in <int> in{"in"};
    sc_out <int> out{"out"};
    sc_signal <int> sig{"sig"};
    deep_bottom1 db0{"db0"};
    SC_CTOR(bottom1) {}
};

SC_MODULE(top) {

    bottom0 b0{"b0"};
    bottom1 b1{"b1"};

    SC_CTOR(top) {
        b0.in(b1.db0.sig);
        b0.out(b1.db0.sig);
        b0.db0.in(b1.sig);
        b0.db0.out(b1.sig);

        b1.db0.in(b0.db0.sig);
        b1.db0.out(b0.db0.sig);
        b1.in(b0.sig);
        b1.out(b0.sig);
    }
};

int sc_main (int argc, char **argv) {
    cout << "test_bind_cross\n";
    top t0{"t0"};
    sc_start();
    return 0;
}

