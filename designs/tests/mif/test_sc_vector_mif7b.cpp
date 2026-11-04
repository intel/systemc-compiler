#include "sct_common.h"
#include <systemc.h>

using Data_t = sct_uint<8>;
using Index_t = sct_uint<2>;

// Capture the selected MIF at call entry, including waits in nested helpers.
struct Mif : sc_module, sc_interface {
    Data_t data_;
    explicit Mif(sc_module_name name) : sc_module(name) {}

    void delayed_put(Data_t value) {
        data_ = 1U;
        wait();
        data_ = value;
    }

    void delay_helper(Data_t value) {
        wait(2);
        data_ = value;
    }

    void nested_put(Data_t value) {
        delay_helper(value);
    }
};

#ifdef MIF_MULTI_SELECTOR
struct Outer : sc_module, sc_interface {
    sc_vector<Mif> SC_NAMED(inner, 4);
    explicit Outer(sc_module_name name) : sc_module(name) {}
};
#endif

struct Dut : sc_module {
    static constexpr unsigned PORTS = 4;
    sc_signal<bool> SC_NAMED(clk);
    sc_signal<bool> SC_NAMED(nrst);
    sc_signal<Index_t> SC_NAMED(index);
    sc_signal<Data_t> SC_NAMED(data);
    sc_signal<Data_t> SC_NAMED(out1);
    sc_signal<Data_t> SC_NAMED(out2);
    sc_signal<Data_t> SC_NAMED(out3);
    sc_signal<Data_t> SC_NAMED(out4);
    sc_vector<Mif> SC_NAMED(entries1, PORTS);
    sc_vector<Mif> SC_NAMED(entries2, PORTS);
    sc_vector<Mif> SC_NAMED(entries3, PORTS);
    sc_vector<Mif> SC_NAMED(entries4, PORTS);
#ifdef MIF_MULTI_SELECTOR
    sc_vector<Outer> SC_NAMED(outer, PORTS);
#endif

    SC_CTOR(Dut) {
        SC_THREAD(run1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);
        SC_THREAD(run2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);
        SC_THREAD(run3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);
        SC_THREAD(run4);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);
#ifdef MIF_MULTI_SELECTOR
        SC_THREAD(multi_selector);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);
#endif
    }

    void run1() {
        for (unsigned slot = 0; slot < PORTS; ++slot) {
            entries1[slot].data_ = 0;
        }
        out1 = 0;
        wait();
        while (true) {
            entries1[index.read()].delayed_put(data.read());
            entries1[Index_t(index.read() + 1U)].delayed_put(data.read());
            out1 = entries1[index.read()].data_;
            wait();
        }
    }

    void run2() {
        for (unsigned slot = 0; slot < PORTS; ++slot) {
            entries2[slot].data_ = 0;
        }
        out2 = 0;
        wait();
        while (true) {
            entries2[index.read()].nested_put(data.read());
            out2 = entries2[index.read()].data_;
            wait();
        }
    }

    void run3() {
        entries3[2].data_ = 0;
        out3 = 0;
        wait();
        while (true) {
            entries3[2].delayed_put(data.read());
            out3 = entries3[2].data_;
            wait();
        }
    }

    void run4() {
        for (unsigned slot = 0; slot < PORTS; ++slot) {
            entries4[slot].data_ = 0;
        }
        entries4[index.read()].delayed_put(data.read());
        out4 = 0;
        while (true) {
            out4 = entries4[index.read()].data_;
            wait();
        }
    }

#ifdef MIF_MULTI_SELECTOR
    void multi_selector() {
        wait();
        while (true) {
            outer[index.read()].inner[index.read()].delayed_put(data.read());
            wait();
        }
    }
#endif
};

int sc_main(int argc, char* argv[]) {
    Dut dut("dut");
    for (unsigned cycle = 0; cycle < 24; ++cycle) {
        dut.clk = false;
        sc_start(5, SC_NS);
        dut.nrst = cycle != 0 && cycle != 15;
        dut.index = cycle % Dut::PORTS;
        dut.data = Data_t(0x20U + cycle);
        dut.clk = true;
        sc_start(5, SC_NS);
    }
    return 0;
}