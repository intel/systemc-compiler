#include "sct_common.h"
#include <systemc.h>

// Paired direct-field/method accesses to independent MIF vectors:
// run1/2: dynamic read/write, no reset.
// run3/4: fixed read, dynamic write, no reset.
// run5/6: dynamic read, separate fixed writes, no reset.
// run7/8: dynamic read/write, with reset.
// run9/10: fixed read/write, with reset.

using Data_t  = sct_uint<8>;
using Index_t = sct_uint<2>;

struct Mif : sc_module, sc_interface
{
    Data_t data_;

    explicit Mif(sc_module_name name) : sc_module(name) {}

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
    sc_signal<Index_t> SC_NAMED(write_index);
    sc_signal<Data_t> SC_NAMED(data);
    sc_signal<Data_t> SC_NAMED(out_data);
    sc_signal<Data_t> SC_NAMED(out_data2);
    sc_signal<Data_t> SC_NAMED(out_data3);
    sc_signal<Data_t> SC_NAMED(out_data4);
    sc_signal<Data_t> SC_NAMED(out_data5);
    sc_signal<Data_t> SC_NAMED(out_data6);
    sc_signal<Data_t> SC_NAMED(out_data7);
    sc_signal<Data_t> SC_NAMED(out_data8);
    sc_signal<Data_t> SC_NAMED(out_data9);
    sc_signal<Data_t> SC_NAMED(out_data10);

    sc_vector<Mif> SC_NAMED(entries1, PORTS);
    sc_vector<Mif> SC_NAMED(entries2, PORTS);
    sc_vector<Mif> SC_NAMED(entries3, PORTS);
    sc_vector<Mif> SC_NAMED(entries4, PORTS);
    sc_vector<Mif> SC_NAMED(entries5, PORTS);
    sc_vector<Mif> SC_NAMED(entries6, PORTS);
    sc_vector<Mif> SC_NAMED(entries7, PORTS);
    sc_vector<Mif> SC_NAMED(entries8, PORTS);
    sc_vector<Mif> SC_NAMED(entries9, PORTS);
    sc_vector<Mif> SC_NAMED(entries10, PORTS);

    SC_CTOR(Dut) {
        SC_THREAD(run1);
        sensitive << clk.posedge_event();

        SC_THREAD(run2);
        sensitive << clk.posedge_event();

        SC_THREAD(run3);
        sensitive << clk.posedge_event();

        SC_THREAD(run4);
        sensitive << clk.posedge_event();

        SC_THREAD(run5);
        sensitive << clk.posedge_event();

        SC_THREAD(run6);
        sensitive << clk.posedge_event();

        SC_THREAD(run7);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);

        SC_THREAD(run8);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);

        SC_THREAD(run9);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);

        SC_THREAD(run10);
        sensitive << clk.posedge_event();
        async_reset_signal_is(nrst, false);
    }

    void run1() {
        while (true) {
            out_data = entries1[index.read()].data_;
            entries1[write_index.read()].data_ = data.read();
            wait();
        }
    }

    void run2() {
        while (true) {
            out_data2 = entries2[index.read()].read();
            entries2[write_index.read()].update(data.read());
            wait();
        }
    }

    void run3() {
        while (true) {
            out_data3 = entries3[1].data_;
            entries3[write_index.read()].data_ = data.read();
            wait();
        }
    }

    void run4() {
        while (true) {
            out_data4 = entries4[1].read();
            entries4[write_index.read()].update(data.read());
            wait();
        }
    }

    void run5() {
        while (true) {
            out_data5 = entries5[index.read()].data_;
            entries5[0].data_ = data.read();
            entries5[1].data_ = Data_t(data.read() + 1U);
            entries5[2].data_ = Data_t(data.read() + 2U);
            entries5[3].data_ = Data_t(data.read() + 3U);
            wait();
        }
    }

    void run6() {
        while (true) {
            out_data6 = entries6[index.read()].read();
            entries6[0].update(data.read());
            entries6[1].update(Data_t(data.read() + 1U));
            entries6[2].update(Data_t(data.read() + 2U));
            entries6[3].update(Data_t(data.read() + 3U));
            wait();
        }
    }

    void run7() {
        for (unsigned slot = 0; slot < PORTS; ++slot) {
            entries7[slot].data_ = 0;
        }
        out_data7 = 0;
        wait();

        while (true) {
            out_data7 = entries7[index.read()].data_;
            entries7[write_index.read()].data_ = data.read();
            wait();
        }
    }

    void run8() {
        for (unsigned slot = 0; slot < PORTS; ++slot) {
            entries8[slot].update(0);
        }
        out_data8 = 0;
        wait();

        while (true) {
            out_data8 = entries8[index.read()].read();
            entries8[write_index.read()].update(data.read());
            wait();
        }
    }

    void run9() {
        for (unsigned slot = 0; slot < PORTS; ++slot) {
            entries9[slot].data_ = 0;
        }
        out_data9 = 0;
        wait();

        while (true) {
            out_data9 = entries9[2].data_;
            entries9[2].data_ = data.read();
            wait();
        }
    }

    void run10() {
        for (unsigned slot = 0; slot < PORTS; ++slot) {
            entries10[slot].update(0);
        }
        out_data10 = 0;
        wait();

        while (true) {
            out_data10 = entries10[2].read();
            entries10[2].update(data.read());
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
        dut.nrst = cycle != 0 && cycle != 7;
        dut.index = cycle % Dut::PORTS;
        dut.write_index = (cycle * 3U + 1U) % Dut::PORTS;
        dut.data = Data_t(0x20U + cycle);
        dut.clk = true;
        sc_start(5, SC_NS);
        if (dut.out_data.read() != dut.out_data2.read() ||
            dut.out_data3.read() != dut.out_data4.read() ||
            dut.out_data5.read() != dut.out_data6.read() ||
            dut.out_data7.read() != dut.out_data8.read() ||
            dut.out_data9.read() != dut.out_data10.read()) {
            SC_REPORT_FATAL("mif_sc_vector_mif7a", "Access scenario mismatch");
        }
    }
    return 0;
}