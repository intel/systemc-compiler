/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Simple Latch and Async Reset Flip flop

using namespace sc_core;

struct Dut : sc_module 
{
    sc_in<bool>          enable{"enable"};       // Enable for Latch
    sc_in<sc_uint<16>>   data_in{"data_in"};     // Data in
    sc_in<bool>          rstn{"rstn"};           // Active low Reset
    sc_out<sc_uint<16>>  q_pos_out{"q_pos_out"}; // Q output for latch

    sc_in<bool>          clk{"clk"};            // Clock for flip flop
    sc_in<bool>          arstn{"arstn"};
    sc_out<sc_uint<16>>  async_rst_dff_out{"async_rst_dff_out"}; // Flip flop Q output


    SC_CTOR(Dut) {
        SC_METHOD(positive_latch); sensitive << enable << data_in << rstn;
        SC_CTHREAD(async_rst_dff, clk.pos());
        async_reset_signal_is(arstn, 0);

    }
    void positive_latch() {
        if (!rstn) {
            q_pos_out = 0;
        } else {
            if (enable) {
                q_pos_out = data_in;
            }
        }
        sct_assert_latch(q_pos_out);
    }
    void async_rst_dff() {
        async_rst_dff_out = 0;
        wait();             // 0

        while (true) {
            async_rst_dff_out = data_in;
            wait();         // 1
        }
    }
};

