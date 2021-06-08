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

    sc_signal<bool> din;

    void forked(unsigned &x1, unsigned &y1) {

        if (din) {
            x1 = 0;
            y1 = 1;
            return;
        } else {
            x1 = 1;
            y1 = 1;
        }
    }

    void forked2(unsigned &x2, unsigned &y2) {

        if (din) {
            x2 = 0;
            y2 ++;
            return;
        } else {
            x2 = 1;
            y2 ++;
        }
    }

    void test_method () {

        unsigned x = 0;
        unsigned y = 0;

        forked(x,y);

        sct_assert_const( y == 1);
        sct_assert_unknown( x == 0);

        if (din)
            forked2(x,y);
        else
            forked2(x,y);

        sct_assert_const( y == 2 );
        sct_assert_unknown( x == 0);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
