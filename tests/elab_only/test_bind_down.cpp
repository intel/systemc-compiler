/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/25/18.
//

#include <systemc.h>


SC_MODULE(deep_bottom) {
    sc_signal<int> sig{"sig"};


    SC_CTOR(deep_bottom) { }
};

SC_MODULE(bottom) {

    sc_in<int> in0{"in0"};
    sc_signal<int> sig{"sig"};

    deep_bottom db{"db"};

    SC_CTOR(bottom) {
        in0.bind(db.sig);
    }

};


SC_MODULE(top) {

    sc_in<int> in0{"in0"};
    sc_in<int> in1{"in1"};
    sc_out<int> out{"out"};

    bottom b{"b"};

    SC_CTOR(top) {
        in0.bind(b.db.sig);
        in1.bind(b.sig);
        out.bind(b.db.sig);
    }
};

int sc_main (int argc, char **argv) {
    cout << "test_bind_down\n";
    top t0{"t0"};
    sc_start();
    return 0;
}

