/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/6/18.
//

#include <systemc.h>
#include <sct_assert.h>


SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_method);
        sensitive << din;
    }

    sc_signal<int> din;

    void test_method () {

        for (size_t k = 0; k < 2; ++k) {

            size_t x = 0;

            for (size_t i = 0; i < 3; ++i) {
                x++;
            }

            sct_assert_const(x==3);

            int i = 0;
            while (i < 3) {
                x++;
                i++;
            }

            sct_assert_const(x==6);

            int j = 0;

            do {
                j++;
                x++;
            } while (j < 3);

            sct_assert_const(x==9);

        }

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
