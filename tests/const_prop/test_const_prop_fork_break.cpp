/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/9/18.
//

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_method);
    }

    sc_signal<bool> din{"din"};

    void test_method() {

        int x = 0;
        int y = 0;
        int z = 0;

        for (size_t i = 0; i < 10; ++i) {
            x++;
            y = 1;
            if (din)
                break;
            z = 1;
        }

        sct_assert_unknown(x);
        sct_assert_unknown(z);
        sct_assert_const(y == 1);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
