/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/10/18.
//

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_method);
        sensitive << din << din2;
    }

    sc_signal<bool> din{"din"};
    sc_signal<bool> din2{"din"};

    void test_method() {

        int x = 0;
        int y = 0;
        int z = 0;

        for (size_t i = 0; i < 3; ++i) {
            x++;
            if (din2) {
                y = 2;
                if (din)
                    continue;
                z = 1;
            } else {
                y = 2;
                z = 1;
            }
        }

        sct_assert_const(x == 3);
        sct_assert_unknown(z);
        sct_assert_const(y == 2);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
