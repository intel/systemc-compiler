/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

//Encoders
//    Encoder - Using if-else Statement
//    Encoder - Using case Statement
// Priority Encoders
//    Pri-Encoder - Using if-else Statement
//    Encoder - Using assign Statement
// Decoders
//    Decoder - Using case Statement
//    Decoder - Using assign Statement

class A : public sc_module 
{
public:
    sc_in<bool>          clk{"clk"};
    sc_in<bool>          enable{"enable"}; // Enable of the encoder
    sc_in<sc_uint<16>>   data_in{"data_in"};
    sc_in<bool>          rstn{"rstn"};
    sc_out<sc_uint<16>>  q_pos_out{"q_pos_out"};
    sc_out<sc_uint<16>>  q_neg_out{"q_neg_out"};
    sc_out<sc_uint<16>>  q_one_out{"q_one_out"};
    sc_out<sc_uint<16>>  q_zero_out{"q_zero_out"};

    sc_out<sc_uint<16>>  async_rst_dff_out{"async_rst_dff_out"};
    sc_out<sc_uint<16>>  sync_rst_dff_out{"sync_rst_dff_out"};
    sc_out<sc_uint<16>>  async_rst_tff_out{"async_rst_tff_out"};
    sc_out<sc_uint<16>>  sync_rst_tff_out{"sync_rst_tff_out"};
    sc_out<sc_uint<16>>  async_rst_tff_out2{"async_rst_tff_out2"};
    sc_out<sc_uint<16>>  async_rst_tff_out3{"async_rst_tff_out3"};



    sc_in<bool> arstn{"arstn"};
    sc_in<bool> sreset{"sreset"};

    SC_CTOR(A) {
        SC_METHOD(positive_latch); sensitive << enable << data_in << rstn;

        SC_METHOD(negative_latch); sensitive << enable << data_in << rstn;

        SC_METHOD(latch_one); sensitive << enable << data_in << rstn;

        SC_METHOD(latch_zero); sensitive << enable << data_in << rstn;

        SC_CTHREAD(async_rst_dff, clk.pos());
        async_reset_signal_is(arstn, 0);

        SC_CTHREAD(sync_rst_dff, clk.pos());
        reset_signal_is(sreset, false);
        SC_CTHREAD(async_rst_tff, clk.pos());
        async_reset_signal_is(arstn, 0);
        SC_CTHREAD(sync_rst_tff, clk.pos());
        reset_signal_is(sreset, false);

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
    void negative_latch() {
        if (!rstn) {
            q_neg_out = 0;
        } else {
            if (!enable) {
                q_neg_out = data_in;
            }
        }
        sct_assert_latch(q_neg_out);
    }
    void latch_one() {
        if (!rstn) {
            q_one_out = 0;
        } else {
            if (enable) {
                q_one_out = 0xFFFF; // {q_one_out.size(){1}};
            }
        }
        sct_assert_latch(q_one_out);
    }
    void latch_zero() {
        if (!rstn) {
            q_zero_out = 0xffff;
        } else {
            if (enable) {
                q_zero_out = 0;
            }
        }
        sct_assert_latch(q_zero_out);
    }

    void async_rst_dff() {
        async_rst_dff_out = 0;
        wait();             // 0

        while (true) {
            async_rst_dff_out = data_in;
            wait();         // 1
        }
    }

    void sync_rst_dff() {
        sync_rst_dff_out = 1;
        wait();             // 0

        while (true) {
            sync_rst_dff_out = data_in;
            wait();         // 1
        }
    }

    void async_rst_tff() {
        sc_uint<16> b = 0;

        async_rst_tff_out = 0x0;
        async_rst_tff_out2 = 0x0;
        async_rst_tff_out3 = 0x0;

        wait();             // 0

        while (true) {
            sc_uint<16> a = async_rst_tff_out.read();
            async_rst_tff_out2 = data_in.read() ^ async_rst_tff_out2.read();

            for (int i = 0; i< 16; i++) {
                if (data_in.read()[i]) {
                    a[i]  = !async_rst_tff_out.read()[i];
                }
            }
            async_rst_tff_out = a;


            for (int i = 0; i< 16; i++) {
                if (data_in.read()[i]) {
                    b[i]  = !async_rst_tff_out3.read()[i];
                }
            }
            async_rst_tff_out3 = b;


            wait();         // 1

      }
    }

    void sync_rst_tff() {
        sc_uint<16> a = 0;

        sync_rst_tff_out = 0x0;

        wait();             // 0

        while (true) {
            a = sync_rst_tff_out.read();

            for (int i = 0; i< 16; i++) {
                if (data_in.read()[i]) {
                    a[i]  = !sync_rst_tff_out.read()[i];
                }
            }
            sync_rst_tff_out = a;
            wait();         // 1

      }
    }


};

struct dut : sc_core::sc_module {
    typedef dut SC_CURRENT_USER_MODULE;
    dut(::sc_core::sc_module_name) {}
};

class B_top : public sc_module
{
public:
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> enable{"enable"};
    sc_signal<sc_uint<16>> data_in{"data_in"};
    sc_signal<bool> rstn{"rstn"};
    sc_signal<bool> arstn{"arstn"};
    sc_signal<bool> sreset{"sreset"};
    sc_signal<sc_uint<16>> q_pos_out{"q_pos_out"};
    sc_signal<sc_uint<16>> q_neg_out{"q_neg_out"};
    sc_signal<sc_uint<16>> q_one_out{"q_one_out"};
    sc_signal<sc_uint<16>> q_zero_out{"q_zero_out"};
    sc_signal<sc_uint<16>>  async_rst_dff_out{"async_rst_dff_out"};
    sc_signal<sc_uint<16>>  sync_rst_dff_out{"sync_rst_dff_out"};
    sc_signal<sc_uint<16>>  async_rst_tff_out{"async_rst_tff_out"};
    sc_signal<sc_uint<16>>  sync_rst_tff_out{"sync_rst_tff_out"};
    sc_signal<sc_uint<16>>  async_rst_tff_out2{"async_rst_tff_out2"};
    sc_signal<sc_uint<16>>  async_rst_tff_out3{"async_rst_tff_out3"};


    A a_mod{"a_mod"};
    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.enable(enable);
        a_mod.data_in(data_in);
        a_mod.rstn(rstn);
        a_mod.arstn(arstn);
        a_mod.sreset(sreset);
        a_mod.q_pos_out(q_pos_out);
        a_mod.q_neg_out(q_neg_out);
        a_mod.q_one_out(q_one_out);
        a_mod.q_zero_out(q_zero_out);
        a_mod.async_rst_dff_out(async_rst_dff_out);
        a_mod.sync_rst_dff_out(sync_rst_dff_out);
        a_mod.async_rst_tff_out(async_rst_tff_out);
        a_mod.sync_rst_tff_out(sync_rst_tff_out);
        a_mod.async_rst_tff_out2(async_rst_tff_out2);
        a_mod.async_rst_tff_out3(async_rst_tff_out3);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

