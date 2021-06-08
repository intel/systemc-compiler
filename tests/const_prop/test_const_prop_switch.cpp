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
        int x = 0;
        x++;
        int y = 0;

        switch (x) {
            case 0:
                y = 1;
                break;
            case 1+1:
                y = 3;
                break;
        }

        sct_assert_const(y == 0);

        switch (x) {
            case 0:
                y+=1;
                break;
            case 1:
                y+=2;
                break;
            case 2:
                y+=3;
                break;
            default:
                y = -1;
                break;
        }

        sct_assert_const(x == 1);
        sct_assert_const(y == 2);

        switch (din.read()) {
            case 0:
                x = 12;
                y = 1;
                break;
            case 1:
                x = 12;
                y = 2;
                break;

            default:
                x = 12;
                y = 2;
                break;
        }

        sct_assert_const(x == 12);
        sct_assert_unknown(y);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
