#include "sct_common.h"
#include "systemc.h"

using namespace sct;

struct Record_t {
    using Bits_t = sct_uint<16>;

    sct_uint<4> tag_;
    sct_uint<12> value_;

    Record_t() : tag_(0u), value_(0u) {}
    explicit Record_t(const Bits_t& bits) { fromBits(bits); }

    void fromBits(const Bits_t& bits) {
        (tag_, value_) = bits;
    }
};

struct RecordHolder_t {
    Record_t::Bits_t bits_;

    Record_t recCtor() const {
        return Record_t(bits_);
    }

    Record_t recFromBits() const {
        Record_t rec;
        rec.fromBits(bits_);
        return rec;
    }

    Record_t recNamed() const {
        Record_t rec(bits_);
        return rec;
    }
};

struct Top : sc_module {
    using Bits_t = Record_t::Bits_t;

    sc_in_clk SC_NAMED(clk);
    sct_in<bool> SC_NAMED(nrst);
    sct_in<Bits_t> SC_NAMED(data);
    sct_out<Bits_t> SC_NAMED(out_a);
    sct_out<Bits_t> SC_NAMED(out_b);
    sct_out<Bits_t> SC_NAMED(out_c);

    SC_CTOR(Top) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);
    }

    void run() {
        RecordHolder_t holder;
        holder.bits_ = 0u;
        out_a = 0u;
        out_b = 0u;
        out_c = 0u;
        wait();

        while (true) {
            holder.bits_ = data.read();
            Record_t a = holder.recCtor();
            Record_t b = holder.recFromBits();
            Record_t c = holder.recNamed();
            out_a = (a.tag_, a.value_);
            out_b = (b.tag_, b.value_);
            out_c = (c.tag_, c.value_);
            wait();
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sct_clock<> SC_NAMED(clk, 1, SC_NS);
    sct_signal<bool> SC_NAMED(nrst);
    sct_signal<Record_t::Bits_t> SC_NAMED(data);
    sct_signal<Record_t::Bits_t> SC_NAMED(out_a);
    sct_signal<Record_t::Bits_t> SC_NAMED(out_b);
    sct_signal<Record_t::Bits_t> SC_NAMED(out_c);
    Top SC_NAMED(top);
    top.clk(clk);
    top.nrst(nrst);
    top.data(data);
    top.out_a(out_a);
    top.out_b(out_b);
    top.out_c(out_c);

#ifndef __SC_TOOL__
    nrst = false;
    data = 0u;
    sc_start(2, SC_NS);
    sc_assert(out_a.read() == 0u);
    sc_assert(out_b.read() == 0u);
    sc_assert(out_c.read() == 0u);
    nrst = true;
    const unsigned patterns[] = {
        0x0001u, 0x1abcu, 0xfedcu, 0x8000u, 0x1234u, 0xffffu, 0x0000u
    };
    for (unsigned pattern : patterns) {
        data = pattern;
        sc_start(2, SC_NS);
        sc_assert(out_a.read() == pattern);
        sc_assert(out_b.read() == pattern);
        sc_assert(out_c.read() == pattern);
    }
#else
    sc_start();
#endif
    return 0;
}