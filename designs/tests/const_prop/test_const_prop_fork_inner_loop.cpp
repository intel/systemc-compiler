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
    sc_signal<bool> din2{"din2"};

    void test_method() {

        int x = 0;
        int y = 0;
        int z = 0;
        int k = 0;

        for (size_t i = 0; i < 10; ++i) {
            x++;
            y = 1;
            if (din) {

                if (din2) {

                    if (i > 0) {
                        x++;
                        k = 1;
                    }

                    size_t j = 0;
                    while ( j < 2) {
                        ++j;
                    }
                }
                x = 3;


                break;
            } else {
                x = 3;
            }
            z = 1;
        }

        sct_assert_const(x == 3);
        sct_assert_unknown(z);
        sct_assert_unknown(k);
        sct_assert_const(y == 1);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
