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
    sc_signal<bool> din2;

    int x;
    int y;

    void wait_wrap () {
        wait();
        x++;

        sct_assert_const(x == 2);
        wait();
        x ++;
        sct_assert_const(x == 3);
    }

    void dwait_wrap () {
        wait();
        x++;
        wait_wrap();
        x++;
    }

    void test_thread() {
        x = 0;
        y = 0;

        dwait_wrap();
        sct_assert_const(x == 4);
        
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
