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

// Behavioral FIFO example
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

    sc_signal<sc_uint<32>> buffer[BUFSIZE];

    sc_uint<LOGBUFSIZE> headp; // FIFO head ptr
    sc_uint<LOGBUFSIZE> tailp; // FIFO tail ptr

    // Counter for fifo depth
    sc_uint<LOGBUFSIZEPLUSONE> num_in_buf;

    SC_CTOR(Circ_buf) {
        SC_HAS_PROCESS(Circ_buf);

        SC_CTHREAD(fifo_rw, clk.pos());
        async_reset_signal_is(reset, true);
    }

    void fifo_rw() {
        // Reset operations
        headp = 0;
        tailp = 0;
        num_in_buf = 0;
        full = false;
        empty = true;
        data_out = 0;
        for (int i = 0; i < BUFSIZE; i++) {
            buffer[i] = 0;
        }
        wait();

        // Main loop
        while (true) {
            if (read_fifo.read()) {
                // Check if FIFO is not empty
                if (num_in_buf != 0) {
                    num_in_buf--;
                    data_out = buffer[headp++];
                    full = false;
                    if (num_in_buf == 0)
                        empty = true;
                }
                // Ignore read request otherwise
            } else if (write_fifo.read()) {
                // Check if FIFO is not full
                if (num_in_buf != BUFSIZE) {
                    buffer[tailp++] = data_in;
                    num_in_buf++;
                    empty = false;
                    if (num_in_buf == BUFSIZE)
                        full = true;
                }
                // Ignore write request otherwise
            } else {
            }
            wait();
        }
    }
};

class B_top: public sc_module
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


