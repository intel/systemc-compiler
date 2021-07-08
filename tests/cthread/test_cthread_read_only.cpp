/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>

// Read-only channels in CTHREAD
SC_MODULE(test_split_array_reg) {

    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};
    sc_in<int> in1;        // RO
    sc_in<int> in2;        // RO

    SC_CTOR(test_split_array_reg) {
        SC_CTHREAD(test_thread, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(test_thread1, clk);
        async_reset_signal_is(rstn, false);
    }

    static constexpr int x = 12;
    sc_signal<int> sig0;
    sc_signal<int> sig1;
    sc_signal<int> ro1;     // RO

    int arr0[2];

    void test_thread() {
        arr0[1] = 0;
        wait();
        while(1) {
            sig0 = sig1 + x + in1.read();
            sct_assert_read(in1);
            sct_assert_defined(in1, false);

            wait();
            sig0 = ro1;
            sct_assert_read(ro1);
            sct_assert_defined(ro1, false);
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
            auto i = in2 + ro1.read();
            sct_assert_read(in2);
            sct_assert_defined(in2, false);
        }
    }

};

int sc_main(int argc, char **argv) {
    sc_signal<int> s;
    test_split_array_reg test{"test"};
    test.in1(s);
    test.in2(s);
    
    sc_start();
    return 0;
}
