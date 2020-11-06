/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 12/14/18.
//

#include <systemc.h>

SC_MODULE(test_split_array_reg) {

    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};

    SC_CTOR(test_split_array_reg) {
        SC_CTHREAD(test_thread, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(test_thread1, clk);
        async_reset_signal_is(rstn, false);
    }

    static constexpr int x = 12;
    sc_signal<int> sig0;
    sc_signal<int> sig1;

    int arr0[2];

    void test_thread() {
        arr0[1] = 0;
        wait();
        while(1) {
            sig0 = sig1 + x;
            wait();
        }
    }

    sc_signal<int> arr[2];

    void test_thread1() {
        sig1 = 0;
        arr[sig0] = 0;
        wait();
        
        while(1) {
            sig1 = x;
            wait();
        }
    }

};

int sc_main(int argc, char **argv) {
    test_split_array_reg test{"test"};
    sc_start();
    return 0;
}
