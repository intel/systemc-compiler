/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/26/18.
//

#include <systemc.h>
#include <sct_assert.h>


SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_method);
        sensitive << din;
    }

    sc_signal<bool> din {"din",1};

    void test_method () {

        for (size_t i = 0; i < 2; ++i) {
        }

        int x = 1;
        if (x) {

        }

        if (din) {
            if(x) {

            }

        } else {
            if (!x) {

            }

        }

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
