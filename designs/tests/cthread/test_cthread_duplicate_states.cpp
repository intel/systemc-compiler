/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// First and last state in generated case are duplicated, for #40
SC_MODULE(dut) {

    sc_signal<bool> clk;
    sc_signal<bool> rstn;

    SC_CTOR(dut) {
        SC_CTHREAD(thread0, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread1, clk);
        async_reset_signal_is(rstn, false);
    }

    sc_signal<bool> ready0;
    sc_signal<bool> valid0;

    sc_signal<bool> ready1;
    sc_signal<bool> valid1;

    sc_signal<bool> ready2;
    sc_signal<bool> valid2;

    void thread0() {
        wait();
        while (1) {

            valid0 = 1;
            wait();
            while (!ready0) {
                wait();
            }
            valid0 = 0;

            wait();
        }
    }

    void thread1() {
        valid1 = 0;
        wait();
        while (1) {

            valid1 = 1;
            do {
                wait();
            } while (!ready1);

            int x = 42;
            wait();
        }
    }



};

int sc_main(int argc, char **argv) {
    dut dut0{"dut0"};
    sc_start();
    return 0;
}
