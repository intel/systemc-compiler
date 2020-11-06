/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/12/18.
//

#include <systemc.h>

struct bottom : sc_module {

    sc_in <int> in{"in"};
    sc_out <int> out{"out"};
    sc_signal <int> sig{"sig"};

    sc_out <int> out_2{"out_2"};

    SC_CTOR(bottom) {
        out.bind(sig);
        in.bind(sig);
    }
};

struct top : sc_module {

    sc_out<int> out{"out"};
    sc_in <int> in0{"in0"};
    sc_in <int> in1{"in1"};

    sc_signal<int> sig_2{"sig_2"};
    sc_out<int> out_2{"sig_2"};
    bottom b{"b"};

    SC_CTOR(top) {
        out(b.out);
        in0(b.in);
        in1(b.in);

        out_2(b.out_2);
        b.out_2(sig_2);

        SC_THREAD(test_thread);
    }

    void test_thread() {
        out = 42;
        out_2 = 43;
        wait(1, SC_NS);

        cout << in0.read() << endl;
        cout << in1.read() << endl;
        cout << sig_2.read() << endl;
    }

};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports 2\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

