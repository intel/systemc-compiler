/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "demo.h"
#include <systemc.h>

using namespace demo;

struct tb : public sc_module {
    static const unsigned N = 16;

    sc_in_clk clk{"clk"};
    sc_in<bool> nrst{"nrst"};

    chan_demo dut{"dut"};
    sct_initiator<int> in_init{"in_init"};
    sct_target<int> out_targ{"out_targ"};

    SC_HAS_PROCESS(tb);

    explicit tb(const sc_module_name& name) : sc_module(name) {
        dut.clk(clk);
        dut.nrst(nrst);

        in_init.clk_nrst(clk, nrst);
        out_targ.clk_nrst(clk, nrst);

        in_init.bind(dut.in_targ);
        out_targ.bind(dut.out_init);

        SC_THREAD(stimulus_thread);
        in_init.addTo(sensitive);
        out_targ.addTo(sensitive);
        async_reset_signal_is(nrst, false);
    }

    void trace(sc_trace_file* file) {
        in_init.trace(file);
        out_targ.trace(file);
        dut.in_targ.trace(file);
        dut.out_init.trace(file);
        dut.data_fifo.trace(file);
    }

    void stimulus_thread() {
        int send_count;
        int recv_count;
        int value;

        in_init.reset_put();
        out_targ.reset_get();
        send_count = 0;
        recv_count = 0;
        wait();

        while (true) {
            in_init.clear_put();

            if (send_count < N && in_init.ready()) {
                in_init.put(send_count);
                send_count++;
            }

            if (out_targ.request()) {
                value = out_targ.get();
                sc_assert(value == recv_count);
                recv_count++;
            }

            if (recv_count == N) {
                sc_stop();
            }

            wait();
        }
    }
};

SC_MODULE(top_env) {
    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<bool> nrst{"nrst"};
    tb tb_i{"tb_i"};

    SC_CTOR(top_env) {
        tb_i.clk(clk);
        tb_i.nrst(nrst);

        SC_THREAD(reset_thread);
        sensitive << clk;
    }

    void trace(sc_trace_file* file) {
        sc_trace(file, clk, "clk");
        sc_trace(file, nrst, "nrst");
        tb_i.trace(file);
    }

    void reset_thread() {
        nrst = false;
        wait(3);
        nrst = true;

        while (true) {
            wait();
        }
    }
};

int sc_main(int argc, char* argv[]) {
    top_env env{"env"};
    sc_trace_file* trace_file = sc_create_vcd_trace_file("copilot_chan_demo");

    env.trace(trace_file);
    sc_start();
    sc_close_vcd_trace_file(trace_file);
    return 0;
}