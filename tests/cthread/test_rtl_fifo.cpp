/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

#define BUFSIZE 4
#define LOGBUFSIZE 2
#define LOGBUFSIZEPLUSONE 3

// BREAK statement in method process body analysis
class Circ_buf : public sc_module
{
public:
    sc_in<bool> clk;
    sc_in<bool> read_fifo;
    sc_in<bool> write_fifo;
    sc_in<sc_uint<32>> data_in;
    sc_in<bool> reset;
    sc_out<sc_uint<32>> data_out;
    sc_out<bool> full;
    sc_out<bool> empty;

    // Internal signals
    sc_signal<sc_uint<32>> buf0, buf0_next;
    sc_signal<sc_uint<32>> buf1, buf1_next;
    sc_signal<sc_uint<32>> buf2, buf2_next;
    sc_signal<sc_uint<32>> buf3, buf3_next;
    sc_signal<sc_uint<LOGBUFSIZEPLUSONE> > num_in_buf, num_in_buf_next;
    sc_signal<bool> full_next, empty_next;
    sc_signal<sc_uint<32>> data_out_next;

    sc_uint<32>                 m;
    sc_uint<32>                 k;
    sc_uint<32>                 n;
    sc_uint<32>*                q;

    SC_CTOR(Circ_buf) {
        SC_HAS_PROCESS(Circ_buf);

        SC_METHOD(ns_logic);
        sensitive << buf0<< buf1<<buf2<<buf3<<read_fifo << write_fifo << data_in << num_in_buf;

        SC_CTHREAD(update_regs, clk.pos());
        async_reset_signal_is(reset, true);

        SC_METHOD(gen_full);
        sensitive << num_in_buf_next;
        SC_METHOD(gen_empty);
        sensitive << num_in_buf_next;
    }
    void gen_full() {
        if (num_in_buf_next.read() == BUFSIZE)
            full_next = 1;
        else
            full_next = 0;
    }
    void gen_empty() {
        if (num_in_buf_next.read() == 0)
            empty_next = 1;
        else
            empty_next = 0;
    }
    void update_regs() {
        full = 0;
        empty = 1;
        num_in_buf = 0;
        buf0 = 0;
        buf1 = 0;
        buf2 = 0;
        buf3 = 0;
        data_out = 0;
        wait();
        while (true) {
            full = full_next;
            empty = empty_next;
            num_in_buf = num_in_buf_next;
            buf0 = buf0_next;
            buf1 = buf1_next;
            buf2 = buf2_next;
            buf3 = buf3_next;
            data_out = data_out_next;
            wait();
        }
    }
    void ns_logic() {
        // Default assignments
        buf0_next = buf0;
        buf1_next = buf1;
        buf2_next = buf2;
        buf3_next = buf3;
        num_in_buf_next = num_in_buf;
        data_out_next = 0;
        if (read_fifo.read() == 1) {
            if (num_in_buf.read() != 0) {
                data_out_next = buf0;
                buf0_next = buf1;
                buf1_next = buf2;
                buf2_next = buf3;
                num_in_buf_next = num_in_buf.read() - 1;
            }
        } else if (write_fifo.read() == 1) {
            switch (sc_uint<32>(num_in_buf.read())) {
            case 0:
                buf0_next = data_in.read();
                num_in_buf_next = num_in_buf.read() + 1;
                break;
            case 1:
                buf1_next = data_in.read();
                num_in_buf_next = num_in_buf.read() + 1;
                break;
            case 2:
                buf2_next = data_in.read();
                num_in_buf_next = num_in_buf.read() + 1;
                break;
            case 3:
                buf3_next = data_in.read();
                num_in_buf_next = num_in_buf.read() + 1;
            default:
                // ignore the write command
                break;
            }
        }
    }


};

class B_top : public sc_module
{
public:
    sc_in<bool> clk{"clk"};
    sc_signal<bool> read_fifo{"read_fifo"};
    sc_signal<bool> write_fifo{"write_fifo"};
    sc_signal<sc_uint<32>> data_in{"data_in"};
    sc_signal<bool> reset{"reset"};
    sc_signal<sc_uint<32>> data_out{"data_out"};
    sc_signal<bool> full{"full"};
    sc_signal<bool> empty{"empty"};

    Circ_buf a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.read_fifo(read_fifo);
        a_mod.write_fifo(write_fifo);
        a_mod.data_in(data_in);
        a_mod.reset(reset);
        a_mod.data_out(data_out);
        a_mod.full(full);
        a_mod.empty(empty);
    }
};

int  sc_main(int argc, char* argv[])
{
    sc_clock clk { "clk", sc_time(1, SC_NS) };

    B_top b_mod{"b_mod"};
    b_mod.clk(clk);
    sc_start();
    return 0;
}


