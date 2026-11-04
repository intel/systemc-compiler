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

// Test for record correct construction in various scopes

struct Record_t {
    sct_uint<16> value = 42u;

    Record_t() = default;
    Record_t(sct_uint<8> high, sct_uint<8> low) {
        set(high, low);
    }

    void set(const sct_uint<8>& high, const sct_uint<8>& low) {
        value = (high, low);
    }

    sct_uint<16> get() const { return value; }
};

struct Top : sc_module {
    using Bits_t = sct_uint<16>;
    static constexpr unsigned LoopCount = 3;

    sc_in_clk SC_NAMED(clk);
    sct_in<bool> SC_NAMED(nrst);
    sct_in<bool> SC_NAMED(request);
    sct_in<Bits_t> SC_NAMED(data);
    sc_vector<sct_out<Bits_t>> SC_NAMED(out_loop, LoopCount);
    sct_out<Bits_t> SC_NAMED(out_function);
    sct_out<Bits_t> SC_NAMED(out_reference);
    sct_out<Bits_t> SC_NAMED(out_nested);
    sct_out<Bits_t> SC_NAMED(out_scope);
    sct_out<Bits_t> SC_NAMED(out_wait);
    sct_out<Bits_t> SC_NAMED(out_constant);

    SC_CTOR(Top) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);

        SC_THREAD(runWait);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);
    }

    Bits_t functionScope(const Bits_t& bits, bool enabled) {
        Record_t record;
        bool valid = false;
        if (enabled) {
            sct_uint<8> high = bits.range(15, 8);
            sct_uint<8> low = bits.range(7, 0);
            record = Record_t(high, low);
            valid = true;
        }
        Bits_t result = 0u;
        if (valid) {
            Bits_t value = record.get();
            sct_assert_unknown(value);
            result = value;
        }
        return result;
    }

    bool fill(Record_t& record, const Bits_t& bits, bool enabled) {
        bool valid = false;
        if (enabled) {
            sct_uint<8> high = bits.range(15, 8);
            sct_uint<8> low = bits.range(7, 0);
            record = Record_t(high, low);
            valid = true;
        }
        return valid;
    }

    bool fillNested(Record_t& record, const Bits_t& bits, bool enabled) {
        bool valid = fill(record, bits, enabled);
        return valid;
    }

    void run() {
        for (unsigned slot = 0; slot < LoopCount; ++slot) {
            out_loop[slot] = 0u;
        }
        out_function = 0u;
        out_reference = 0u;
        out_nested = 0u;
        out_scope = 0u;
        out_constant = 0u;
        wait();

        while (true) {
            Bits_t bits = data.read();
            for (unsigned slot = 0; slot < LoopCount; ++slot) {
                Record_t record;
                bool valid = false;
                if (request.read()) {
                    Bits_t shifted = bits + slot;
                    sct_uint<8> high = shifted.range(15, 8);
                    sct_uint<8> low = shifted.range(7, 0);
                    record = Record_t(high, low);
                    valid = true;
                }
                if (valid) {
                    Bits_t value = record.get();
                    sct_assert_unknown(value);
                    out_loop[slot] = value;
                }
            }

            Record_t reference;
            Record_t nested;
            bool reference_valid = false;
            bool nested_valid = false;
            if (request.read()) {
                reference_valid = fill(reference, bits, request.read());
                nested_valid = fillNested(nested, bits, request.read());
                out_function = functionScope(bits, request.read());
            }
            if (reference_valid) {
                Bits_t value = reference.get();
                sct_assert_unknown(value);
                out_reference = value;
            }
            if (nested_valid) {
                Bits_t value = nested.get();
                sct_assert_unknown(value);
                out_nested = value;
            }

            Record_t scoped;
            bool scope_valid = false;
            if (request.read()) {
                sct_uint<8> high = bits.range(15, 8);
                sct_uint<8> low = bits.range(7, 0);
                if (bits[0]) {
                    scoped = Record_t(low, high);
                } else {
                    scoped = Record_t(high, low);
                }
                scope_valid = true;
            }
            if (scope_valid) {
                Bits_t value = scoped.get();
                sct_assert_unknown(value);
                out_scope = value;
            }

            if (request.read()) {
                Record_t constant;
                constant = Record_t(0x12u, 0x34u);
                Bits_t value = constant.get();
                sct_assert_const(value == 0x1234u);
                out_constant = value;
            }
            wait();
        }
    }

    void runWait() {
        Record_t record;
        out_wait = 0u;
        wait();

        while (true) {
            bool valid = false;
            if (request.read()) {
                Bits_t bits = data.read();
                sct_uint<8> high = bits.range(15, 8);
                sct_uint<8> low = bits.range(7, 0);
                record = Record_t(high, low);
                valid = true;
            }
            wait();
            if (valid) {
                Bits_t value = record.get();
                sct_assert_unknown(value);
                sct_assert_register(record.value);
                out_wait = value;
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
    sc_vector<sct_signal<Top::Bits_t>>
        SC_NAMED(out_loop, Top::LoopCount);
    sct_signal<Top::Bits_t> SC_NAMED(out_function);
    sct_signal<Top::Bits_t> SC_NAMED(out_reference);
    sct_signal<Top::Bits_t> SC_NAMED(out_nested);
    sct_signal<Top::Bits_t> SC_NAMED(out_scope);
    sct_signal<Top::Bits_t> SC_NAMED(out_wait);
    sct_signal<Top::Bits_t> SC_NAMED(out_constant);
    Top SC_NAMED(top);
    top.clk(clk);
    top.nrst(nrst);
    top.request(request);
    top.data(data);
    for (unsigned slot = 0; slot < Top::LoopCount; ++slot) {
        top.out_loop[slot](out_loop[slot]);
    }
    top.out_function(out_function);
    top.out_reference(out_reference);
    top.out_nested(out_nested);
    top.out_scope(out_scope);
    top.out_wait(out_wait);
    top.out_constant(out_constant);

#ifndef __SC_TOOL__
    unsigned expected = 0u;
    bool have_data = false;
    auto check = [&]() {
        for (unsigned slot = 0; slot < Top::LoopCount; ++slot) {
            Top::Bits_t loop_expected = have_data ?
                Top::Bits_t(expected + slot) : Top::Bits_t(0u);
            sc_assert(out_loop[slot].read() == loop_expected);
        }
        unsigned scope_expected = expected;
        if ((expected & 1u) != 0u) {
            scope_expected = ((expected & 0xffu) << 8) | (expected >> 8);
        }
        sc_assert(out_function.read() == expected);
        sc_assert(out_reference.read() == expected);
        sc_assert(out_nested.read() == expected);
        sc_assert(out_scope.read() == scope_expected);
        sc_assert(out_wait.read() == expected);
        sc_assert(out_constant.read() == (have_data ? 0x1234u : 0u));
    };

    nrst = false;
    request = false;
    data = 0u;
    sc_start(4, SC_NS);
    check();
    nrst = true;
    const unsigned patterns[] = {
        0x1234u, 0xabcdu, 0xffffu, 0x0102u, 0u, 0x8001u, 0x0042u
    };
    unsigned cycle = 0u;
    for (unsigned pattern : patterns) {
        bool enabled = cycle % 3u != 1u;
        request = enabled;
        data = pattern;
        if (enabled) {
            expected = pattern;
            have_data = true;
        }
        sc_start(4, SC_NS);
        check();
        ++cycle;
    }
    nrst = false;
    expected = 0u;
    have_data = false;
    sc_start(2, SC_NS);
    check();
#else
    sc_start();
#endif
    return 0;
}