/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 8/17/18.
//

#include <systemc.h>

SC_MODULE(test) {

    sc_signal<int> dummy{"dummy",10};

    SC_CTOR(test) {

        SC_METHOD(for_method);
        sensitive << dummy;

        SC_METHOD(while_method);
        sensitive << dummy;

        SC_METHOD(do_method);
        sensitive << dummy;

    }

    void for_method() {
        for (size_t i = 0; i < dummy; ++i) {
        }
    }

    void while_method() {
        bool b;
        while (b) {
            b = false;
        }
    }

    void do_method() {
        bool b;
        do {
            if (b)
                b = !b;
        } while (b);
    }
    
};

int sc_main (int argc, char **argv) {

    test t_inst{"t_inst"};
    sc_start();

    return EXIT_SUCCESS;
}
