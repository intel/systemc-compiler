/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 10/18/18.
//

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(test_module) {

    static constexpr int LINE_INDEX_NUM = 10;
    static constexpr int CACHE_WAY_NUM = 10;
    static constexpr int PORT_NUM = 10;

    bool updateLru[PORT_NUM+1];
    bool updateDirty[PORT_NUM+1];

    sc_int<32> updateLine[PORT_NUM+1];
    sc_int<32> updateWay[PORT_NUM+1];
    sc_int<32> lruMemory[LINE_INDEX_NUM][CACHE_WAY_NUM];

    bool dirtyMemory[LINE_INDEX_NUM][CACHE_WAY_NUM];

    sc_in<bool> clk;
    
    SC_CTOR(test_module) {
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(din, 0);
    }

    sc_signal<bool> din;
    
    void test_thread() {

        int x;

        for (int i = 0; i < LINE_INDEX_NUM; i++) {

            for (int j = 0; j < CACHE_WAY_NUM; j++) {
                bool b = 0;

                for (unsigned port = 0; port < PORT_NUM+1; port++) {
                    b = b || updateLru[port] && updateLine[port] == i &&
                        updateWay[port] == j;
                }
                if (b) lruMemory[i][j] = 0;

                bool d = 0;

                for (unsigned port = 0; port < PORT_NUM; port++) {
                    d = d || updateDirty[port] && updateLine[port] == i &&
                        updateWay[port] == j;
                }
                if (d) dirtyMemory[i][j] = 1;
                if (updateDirty[PORT_NUM] && updateLine[PORT_NUM] == i &&
                    updateWay[PORT_NUM] == j) dirtyMemory[i][j] = 0;

                if (b&&d)
                    x = 12;
                else
                    x = 12;
            }
        }

        sct_assert_const(x == 12);
    }

};


int sc_main(int argc, char **argv) {
    auto tmod = std::make_unique<test_module>("tmod");
    sc_clock    clk{"clk", 10 , SC_NS};
    tmod->clk(clk);
    return 0;
}

