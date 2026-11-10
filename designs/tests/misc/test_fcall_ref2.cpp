#include <sct_common.h>

using namespace sc_core;
using namespace sct;

struct Helpers {
    static sct_uint<8> mask(sct_uint<2> position) {
        return sct_uint<8>(0x0fu) << position;
    }

    static sct_uint<8> pick(sct_uint<8> previous, sct_uint<8> data,
                            sct_uint<8> enable) {
        return enable[0] ? data : previous;
    }
};

struct Dut : sc_module {
    sct_in<bool> SC_NAMED(clk);
    sct_in<bool> SC_NAMED(nrst);
    sct_in<sct_uint<2>> SC_NAMED(position);
    sct_in<sct_uint<8>> SC_NAMED(data);
    sct_out<sct_uint<8>> SC_NAMED(out);

    SC_HAS_PROCESS(Dut);

    explicit Dut(sc_module_name name) : sc_module(name) {
        SC_THREAD(run);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);
    }

    void run() {
        sct_uint<8> result = 0;
        out = 0;
        wait();

        while (true) {
            const sct_uint<2> index = position.read();
            const sct_uint<8> enable = Helpers::mask(index);
            const sct_uint<8> value = data.read();
#if defined(FCALL_REF2_THEN)
            if (index != 1) {
                result = Helpers::pick(result, value, enable);
            } else {
                result = 0;
            }
#elif defined(FCALL_REF2_AFTER_IF)
            if (index == 1) {
                result = 0;
            }
            result = Helpers::pick(result, value, enable);
#else
            if (index == 1) {
                result = 0;
            } else {
                result = Helpers::pick(result, value, enable);
            }
#endif
            out = result;
            wait();
        }
    }
};

int sc_main(int argc, char** argv) {
    sct_clock<> SC_NAMED(clk, 1, SC_NS);
    sct_signal<bool> SC_NAMED(nrst);
    sct_signal<sct_uint<2>> SC_NAMED(position);
    sct_signal<sct_uint<8>> SC_NAMED(data);
    sct_signal<sct_uint<8>> SC_NAMED(out);
    Dut SC_NAMED(top);
    top.clk(clk);
    top.nrst(nrst);
    top.position(position);
    top.data(data);
    top.out(out);

#ifndef __SC_TOOL__
    nrst = false;
    sc_start(2, SC_NS);
    sc_assert(out.read() == 0u);
    nrst = true;

    const unsigned positions[] = {0, 2, 3, 1, 2, 0, 1, 0};
    const unsigned inputs[] = {0x5a, 0xa5, 0xff, 0x11, 0xff, 0xff, 0, 0x81};
    const unsigned expected[] = {0x5a, 0x5a, 0x5a, 0, 0, 0xff, 0, 0x81};
    for (unsigned index = 0; index < 8; ++index) {
        position = positions[index];
        data = inputs[index];
        sc_start(2, SC_NS);
        sc_assert(out.read() == expected[index]);
    }

    nrst = false;
    sc_start(2, SC_NS);
    sc_assert(out.read() == 0u);
#else
    sc_start();
#endif
    return 0;
}