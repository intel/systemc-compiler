/******************************************************************************
* Copyright (c) 2026, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include <systemc.h>

// Read at dynamically indexed MIF array entries and check reset/write statements
// are not removed

using Data_t  = sct_uint<8>;
using Index_t = sct_uint<2>;

struct Mif : sc_module, sc_interface
{
    Data_t data_;

    explicit Mif(sc_module_name name) : sc_module(name) {}

    void reset() {
        data_ = 0;
    }

    void update(Data_t val) {
        data_ = val;
    }

    Data_t read() const { return data_; }
};

struct Dut : sc_module
{
    static constexpr unsigned PORTS = 4;

    sc_signal<bool> SC_NAMED(clk);
    sc_signal<bool> SC_NAMED(nrst);
    sc_signal<Index_t> SC_NAMED(index);
    sc_signal<Data_t> SC_NAMED(out_data);

    sc_vector<Mif> SC_NAMED(entries1, PORTS);

    SC_CTOR(Dut) {
        SC_THREAD(run);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);
    }

    void run() {
        for (unsigned i = 0; i < PORTS; ++i) {
            entries1[i].data_ = 1U;
        }
        out_data = 0;
        wait();

        while (true) {
            // Read the data from the dynamically indexed MIF entry
            out_data = entries1[index.read()].data_;

            for (unsigned i = 0; i < PORTS; ++i) {
                entries1[i].data_ = i;
            }
            wait();
        }
    }
};

int sc_main(int argc, char* argv[])
{
    Dut dut("dut");
    for (unsigned cycle = 0; cycle < 12; ++cycle) {
        dut.clk = false;
        sc_start(5, SC_NS);
        dut.nrst = cycle != 0;
        dut.index = cycle % Dut::PORTS;
        dut.clk = true;
        sc_start(5, SC_NS);
    }
    return 0;
}