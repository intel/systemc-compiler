/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 8/16/18.
//

#include <systemc.h>

struct inner : sc_module {
    sc_in<int> din{"din"};
    SC_CTOR(inner) {}
};

struct inner2 : sc_module {
    sc_in<int> din2{"din2"};
    SC_CTOR(inner2) {}
};

struct dut : sc_module {
    sc_signal<int> sig{"sig"};
    SC_CTOR(dut) {}
};

struct inner3 : sc_module {
    sc_in<int> din3{"din2"};

    dut dut_0{"dut_0"};
    SC_CTOR(inner3) {
        din3(dut_0.sig);
    }
};

struct inner_deep : sc_module {

    inner inner_0{"inner_0"};

    SC_CTOR(inner_deep) {
    }
};

struct dut_deep : sc_module {

    dut dut_0{"dut_0"};

    SC_CTOR(dut_deep) {

    }
};


struct top : sc_module {

    inner inner_0{"inner_0"};
    inner inner_1{"inner_1"};
    inner2 inner2_0{"inner2_0"};

    dut dut0{"dut0"};
    dut dut1{"dut1"};
    dut dut2{"dut2"};

    inner3 inner3_0{"inner3_0"};

    inner_deep inner_deep_0{"inner_deep_0"};
    dut_deep dut_deep_0{"dut_deep_0"};

    top(sc_module_name) {
        inner_0.din(dut0.sig);
        inner_1.din(dut1.sig);
        inner2_0.din2(dut2.sig);

        inner_deep_0.inner_0.din(dut_deep_0.dut_0.sig);
    }

};

int sc_main(int argc, char **argv) {
    auto t_inst = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

