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
    sc_in<bool> clk;
    sc_signal<bool> nrst;

    SC_CTOR(top) {
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    sc_signal<bool> din;


    void wrapstate () {


        for (size_t i = 0; i < 4; ++i) {
            wait();
        }


    }

    void test_thread() {

        while (1) {

            for (size_t i = 0; i < 3; ++i) {
                wait();
            }

            for (size_t j = 0; j < 3; ++j) {
                int x = 0;
                while (x < 3) {
                    x++;
                    wait();
                }

            }

            wrapstate();

            int k = 0;

            while (k > 0)  {
                wait();
                k++;
            }

            wait();

        }
    }

};

int sc_main (int argc, char ** argv ) {

    sc_clock    clk{"clk", 10 , SC_NS};
    top t_inst{"t_inst"};
    t_inst.clk(clk);
    sc_start(400, SC_NS);

    return 0;
}
