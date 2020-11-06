/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constant/constexpr static/non-static array in global scope and class/record scope
struct S {
    static constexpr int ARR1[2] = {10,11};
    static const int ARR2[2];
};
const int S::ARR2[2] = {21,22};

static constexpr int ARR3[2] = {30,31};
const int ARR4[2] = {40,41};

SC_MODULE(test) {

    static constexpr int ARR5[2] = {50,51};
    static const int ARR6[2];
    const int ARR7[2] = {70,71};

    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};

    SC_CTOR(test) {
        SC_METHOD(test_method); sensitive << clk << idx;
        
        SC_CTHREAD(test_thread, clk);
        async_reset_signal_is(rstn, false);
    }

    void test_method() {
        mouts = S::ARR1[idx.read()];
        mouts = S::ARR2[idx.read()];
        mouts = ARR3[idx.read()];
        mouts = ARR4[idx.read()];
        mouts = ARR5[idx.read()];
        mouts = ARR6[idx.read()];
        mouts = ARR7[idx.read()];
    }

    sc_signal<bool> idx{"idx"};
    sc_signal<int> outs{"outs"};
    sc_signal<int> mouts{"mouts"};

    void test_thread() {
        outs = S::ARR1[idx.read()];
        wait();
        outs = S::ARR2[idx.read()];

        while (1) {
            outs = ARR3[idx.read()];
            outs = ARR4[idx.read()];
            outs = ARR5[idx.read()];
            outs = ARR6[idx.read()];
            outs = ARR7[idx.read()];
            wait();
        }
    }
};

const int test::ARR6[2] = {61,62};


int sc_main(int argc, char **argv) {

    test test_inst{"test_inst"};
    sc_start();

    return 0;
}
