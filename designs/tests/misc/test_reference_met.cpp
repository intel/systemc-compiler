/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 10/3/18.
//

#include <systemc.h>

SC_MODULE(test_referece_met) {

    sc_signal<bool> sig{"sig"};
    sc_signal<bool> sigArray[2];

    int x;
    int &xref = x;


    SC_CTOR(test_referece_met) {
        SC_METHOD(test_method);
        sensitive << sig;
    }

    void test_method() {
        x = sig.read();
        xref = 2;
        sigArray[0] = 1;
    }

};

int sc_main(int argc, char **argv) {
    test_referece_met tinst{"tinst"};
    sc_start();
    return 0;
}
