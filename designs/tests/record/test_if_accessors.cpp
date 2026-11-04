/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "sct_assert.h"
#include "systemc.h"

using namespace sct;

// Test for record correct construction in if-scope

struct Record_t {
    sct_uint<16> value = 42u;

    Record_t() = default;
    Record_t(sct_uint<8> high, sct_uint<8> low) {
        value = (high, low);
    }

    sct_uint<16> get() const { return value; }
};

struct Top : sc_module {
    using Bits_t = sct_uint<16>;

    sc_in_clk SC_NAMED(clk);
    sct_in<bool> SC_NAMED(nrst);
    sct_in<bool> SC_NAMED(request);
    sct_in<Bits_t> SC_NAMED(data);
    sct_out<Bits_t> SC_NAMED(out);

    SC_CTOR(Top) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);
    }

    void run() {
        out = 0u;
        wait();

        while (true) {
            Record_t record;
            bool valid = false;
            if (request.read()) {
                Bits_t bits = data.read();
                sct_uint<8> high = bits.range(15, 8);
                sct_uint<8> low = bits.range(7, 0);
                record = Record_t(high, low);
                valid = true;
            }
            if (valid) {
                Bits_t value = record.get();
                sct_assert_unknown(value);
                out = value;
            }
            wait();
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sct_clock<> SC_NAMED(clk, 1, SC_NS);
    sct_signal<bool> SC_NAMED(nrst);
    sct_signal<bool> SC_NAMED(request);
    sct_signal<Top::Bits_t> SC_NAMED(data);
    sct_signal<Top::Bits_t> SC_NAMED(out);
    Top SC_NAMED(top);
    top.clk(clk);
    top.nrst(nrst);
    top.request(request);
    top.data(data);
    top.out(out);

#ifndef __SC_TOOL__
    nrst = false;
    request = false;
    data = 0u;
    sc_start(2, SC_NS);
    sc_assert(out.read() == 0u);
    nrst = true;
    request = true;
    const unsigned patterns[] = {0x1234u, 0xabcdu, 0xffffu};
    for (unsigned pattern : patterns) {
        data = pattern;
        sc_start(2, SC_NS);
        sc_assert(out.read() == pattern);
    }
    request = false;
    data = 0u;
    sc_start(2, SC_NS);
    sc_assert(out.read() == 0xffffu);
#else
    sc_start();
#endif
    return 0;
}