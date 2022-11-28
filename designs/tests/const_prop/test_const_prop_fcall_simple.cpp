/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/15/18.
//

#include <systemc.h>
#include <sct_assert.h>


SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_method);
        sensitive << din;
    }

    sc_signal<bool> din {"din",1};

    int getinc(int x1) {
        return x1 + 1;
    }

    int incr(int &x0) {
        x0 ++;
        return getinc(x0);
    }


    void test_method () {
        int x = 0;

        int y = incr(x);

        sct_assert_const( x == 1);
        sct_assert_const( y == 2);

        y = incr(x);

        sct_assert_const( x == 2);
        sct_assert_const( y == 3);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
