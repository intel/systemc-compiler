/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/11/18.
//

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(top) {

    sc_in<bool> clk;
    sc_signal<bool> nrst;

    SC_CTOR(top) {
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    sc_signal<bool> din;

    void test_thread() {

        while (1) {

            int x = 1;
            x++;

            if (x == 1)
                wait();

            for (size_t i = 2; i > x; --i) {
                wait();
            }

            while (x == 1) {
                wait();
                x++;
            }

            if (din)
                wait();


            wait();
        }
    }

};

int sc_main (int argc, char ** argv ) {

    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(400, SC_NS);

    return 0;
}
