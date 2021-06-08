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

    sc_signal<bool> din {"din",1};

    void test_method () {

        int x = 0;
        int y = 1;

        do {
            x++;
            y++;
        } while (x < 3);

        sct_assert_const(x == 3);
        sct_assert_const(y == 4);


        do {
            y = 3;
            if (x);
                break;
            x++;
        } while (din.read());


        sct_assert_const(y == 3);
        //sct_assert_unknown(x);
    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
