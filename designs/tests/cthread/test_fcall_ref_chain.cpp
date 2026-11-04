#include "sct_common.h"
#include "sct_assert.h"
#include "systemc.h"

using namespace sct;

struct Top : sc_module {
    using Data_t = sct_uint<16>;

    sc_in_clk SC_NAMED(clk);
    sct_in<bool> SC_NAMED(nrst);
    sct_in<Data_t> SC_NAMED(data);
    sct_out<Data_t> SC_NAMED(out_nested);
    sct_out<Data_t> SC_NAMED(out_alias);
    sct_out<Data_t> SC_NAMED(out_direct);

    Data_t pend_nested;
    Data_t pend_alias;
    Data_t pend_direct;

    SC_CTOR(Top) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);
    }

    void writeInner(const Data_t& value) {
        out_nested.write(value);
    }

    void writeNested(const Data_t& value) {
        writeInner(value);
    }

    void writeAlias(const Data_t& value) {
        const Data_t& alias = value;
        out_alias.write(alias);
    }

    void run() {
        pend_nested = 0u;
        pend_alias = 0u;
        pend_direct = 0u;
        out_nested = 0u;
        out_alias = 0u;
        out_direct = 0u;
        wait();

        while (true) {
            writeNested(pend_nested);
            writeAlias(pend_alias);
            out_direct = pend_direct;
            sct_assert_register(pend_nested);
            sct_assert_register(pend_alias);
            sct_assert_register(pend_direct);
            pend_nested = data.read();
            pend_alias = data.read();
            pend_direct = data.read();
            wait();
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sct_clock<> SC_NAMED(clk, 1, SC_NS);
    sct_signal<bool> SC_NAMED(nrst);
    sct_signal<Top::Data_t> SC_NAMED(data);
    sct_signal<Top::Data_t> SC_NAMED(out_nested);
    sct_signal<Top::Data_t> SC_NAMED(out_alias);
    sct_signal<Top::Data_t> SC_NAMED(out_direct);
    Top SC_NAMED(top);
    top.clk(clk);
    top.nrst(nrst);
    top.data(data);
    top.out_nested(out_nested);
    top.out_alias(out_alias);
    top.out_direct(out_direct);

#ifndef __SC_TOOL__
    nrst = false;
    data = 0u;
    sc_start(2, SC_NS);
    sc_assert(out_nested.read() == 0u);
    sc_assert(out_alias.read() == 0u);
    sc_assert(out_direct.read() == 0u);
    nrst = true;
    const unsigned patterns[] = {
        0x0001u, 0x1abcu, 0xfedcu, 0x8000u, 0x1234u, 0xffffu, 0x0000u
    };
    unsigned previous = 0u;
    for (unsigned pattern : patterns) {
        data = pattern;
        sc_start(1, SC_NS);
        sc_assert(out_nested.read() == previous);
        sc_assert(out_alias.read() == previous);
        sc_assert(out_direct.read() == previous);
        previous = pattern;
    }
    sc_start(1, SC_NS);
    sc_assert(out_nested.read() == previous);
    sc_assert(out_alias.read() == previous);
    sc_assert(out_direct.read() == previous);
#else
    sc_start();
#endif
    return 0;
}