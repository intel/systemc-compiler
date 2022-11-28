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

template <int IVAL>
struct dut : sc_module {

    sc_in<bool> din{"din"};

    sc_signal<bool> internal{"internal"};

    dut(sc_module_name, int param) {
        SC_HAS_PROCESS(dut);

        if (param) {
            SC_METHOD(dut_method0);
        } else if (IVAL) {
            SC_METHOD(dut_method1);
        } else {
            SC_METHOD(dut_method2);
        }

        if (param > 1) {
            sensitive << din;
        } else {
            sensitive << internal;
        }
    }

    void dut_method0() {
        std::cout << "dut_method0\n";
    }

    void dut_method1() {
        std::cout << "dut_method1\n";
    }

    void dut_method2() {
        std::cout << "dut_method2\n";
    }

};

struct top : sc_module {

    sc_signal<bool> din{"din"};

    dut<1> dut0{"dut0", 0};
    dut<1> dut1{"dut1", 1};
    dut<1> dut2{"dut2", 2};
    dut<1> dut3{"dut3", 0};
    dut<0> dut4{"dut4", 0};
    dut<0> dut5{"dut5", 1};
    dut<0> dut6{"dut6", 2};
    dut<0> dut7{"dut7", 3};

    top(sc_module_name) {
        dut0.din(din);
        dut1.din(din);
        dut2.din(din);
        dut3.din(din);
        dut4.din(din);
        dut5.din(din);
        dut6.din(din);
        dut7.din(din);
    }

};

int sc_main(int argc, char **argv) {
    auto t_inst = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}


