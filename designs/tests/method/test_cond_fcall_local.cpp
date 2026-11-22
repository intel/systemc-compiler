/******************************************************************************
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Function call in a branch of conditional operator, where the function
// writes only its own local variables: a bit of a local variable, a range of
// a local variable, a local variable through a reference parameter of a
// function it calls, and the fields of a local record, passed by value to
// a function it calls. None of them is a side effect of the function, so no
// error is reported (issue #95).
class A : public sc_module {
public:
    sc_in<bool>         sel{"sel"};
    sc_in<sc_uint<8>>   a{"a"};
    sc_out<sc_uint<8>>  y1{"y1"};
    sc_out<sc_uint<8>>  y2{"y2"};
    sc_out<sc_uint<8>>  y3{"y3"};
    sc_out<sc_uint<8>>  y4{"y4"};

    SC_HAS_PROCESS(A);
    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(bit_write); sensitive << sel << a;
        SC_METHOD(range_write); sensitive << sel << a;
        SC_METHOD(ref_write); sensitive << sel << a;
        SC_METHOD(record_param); sensitive << sel << a;
    }

    static sc_uint<8> reverse(sc_uint<8> x) {
        sc_uint<8> r = 0;
        for (unsigned i = 0; i < 8; ++i) r[i] = x[7 - i];
        return r;
    }

    static sc_uint<8> swap(sc_uint<8> x) {
        sc_uint<8> r = 0;
        r.range(3, 0) = x.range(7, 4);
        r.range(7, 4) = x.range(3, 0);
        return r;
    }

    static void inc(sc_uint<8> x, sc_uint<8>& o) { o = x + 1; }
    static sc_uint<8> inc_local(sc_uint<8> x) {
        sc_uint<8> t;
        inc(x, t);
        return t;
    }

    struct Pair {
        sc_uint<4> lo;
        sc_uint<4> hi;
    };
    static sc_uint<8> join(Pair p) { return (p.hi, p.lo); }
    static sc_uint<8> swap_pair(sc_uint<8> x) {
        Pair p;
        p.lo = x.range(7, 4);
        p.hi = x.range(3, 0);
        return join(p);
    }

    // Bits of a local variable written
    void bit_write() {
        y1 = sel.read() ? reverse(a.read()) : a.read();
    }

    // Ranges of a local variable written
    void range_write() {
        y2 = sel.read() ? a.read() : swap(a.read());
    }

    // Local variable written through a reference parameter of called function
    void ref_write() {
        y3 = sel.read() ? inc_local(a.read()) : a.read();
    }

    // Fields of a local record written, record passed by value
    void record_param() {
        y4 = sel.read() ? swap_pair(a.read()) : a.read();
    }
};

int sc_main(int argc, char** argv) {
    sc_signal<bool> sel;
    sc_signal<sc_uint<8>> a, y1, y2, y3, y4;
    A top("top");
    top.sel(sel);
    top.a(a);
    top.y1(y1);
    top.y2(y2);
    top.y3(y3);
    top.y4(y4);
    sc_start();
    return 0;
}
