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

    sc_in<bool> clk;

    SC_CTOR(top) {
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(din, 0);
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
        wait();
        
        while (true) wait();

    }

};

int sc_main (int argc, char ** argv ) {

    sc_clock    clk{"clk", 10 , SC_NS};
    top t_inst{"t_inst"};
    t_inst.clk(clk);
    sc_start(100, SC_NS);

    return 0;
}

