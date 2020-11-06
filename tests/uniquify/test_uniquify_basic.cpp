/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/25/18.
//

#include <systemc.h>

SC_MODULE(inner) {
    sc_signal<bool> dummy{"dummy"};
    SC_CTOR(inner) {}
};

template <int IVAL>
struct dut : sc_module {

    sc_in<bool> din{"din"};
    const int id;

    inner inner_inst{"inner_inst"};

    dut(sc_module_name, int param) : id(param) {
        SC_HAS_PROCESS(dut);
        SC_METHOD(dut_method);
    }

    void dut_method() {
        std::cout << "Hello from dut<" << IVAL << ">(" << id << ")" << endl;
    }

};

struct top : sc_module {

    sc_signal<bool> din{"din"};

    dut<1> dut0{"dut0", 1};
    dut<2> dut1{"dut1", 1};
    dut<1> dut2{"dut2", 2};
    dut<1> dut3{"dut3", 1};

    top(sc_module_name) {
        dut0.din(din);
        dut1.din(din);
        dut2.din(din);
        dut3.din(din);
    }

};

int sc_main(int argc, char **argv) {
    auto t_inst = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}


