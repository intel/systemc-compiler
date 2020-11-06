/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/13/18.
//

#include <systemc.h>

struct deep : sc_module {

    sc_in_clk clk{"clk"};
    sc_out<int> out{"out"};

    SC_CTOR(deep) {
    }

};

struct bottom : sc_module {

    sc_in_clk clk{"clk"};
    deep d{"d"};

    SC_CTOR(bottom) {
        d.clk(clk);
    }
};

struct top : sc_module {

    sc_signal<bool> clk{"clk"};
    sc_signal<int> sig{"sig"};
    sc_in<int> in{"in"};
    bottom b{"b"};

    SC_CTOR(top) {

        b.clk(clk);
        in.bind(b.d.out);
        b.d.out.bind(sig);

        SC_THREAD(test_thread);
    }

    void test_thread() {
        sig = 12;
        wait(1,SC_NS);
        cout << in << endl;
    }

};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports 3\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

