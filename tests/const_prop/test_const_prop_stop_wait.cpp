/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/8/18.
//

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(top) {

    sc_clock    clk_gen{"clk_gen", 10 , SC_NS};

    SC_CTOR(top) {
        SC_THREAD(test_thread);
        sensitive << clk_gen.posedge_event();
    }

    sc_signal<bool> din;

    int x;

    void test_thread() {
        x = 0;
        int y = 2;

        if (din) {
            x++;
            y++;
            sct_assert_const(x == 1);
            sct_assert_const(y == 3);
            wait();
        }
        else {
            x++;
        }

//        sct_assert_const(x == 1);
//        sct_assert_const(y == 2);
    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start(100, SC_NS);

    return 0;
}

