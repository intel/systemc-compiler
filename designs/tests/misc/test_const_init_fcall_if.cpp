/******************************************************************************
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// A constant initialized by a function call, passed to another function call
// after an if: in the else branch, in the then branch, and after the if.
// The if clears the stored result of the first call, and the value of the
// constant is not known, so it must not be parsed again from its initializer.
struct H {
    static sc_uint<8> mask(sc_uint<2> p) {
        return sc_uint<8>(0xf) << p;
    }
    static sc_uint<8> pick(sc_uint<8> old, sc_uint<8> in, sc_uint<8> en) {
        return en[0] ? in : old;
    }
};

class A : public sc_module {
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    sc_in<sc_uint<2>>   in_p{"in_p"};
    sc_in<sc_uint<8>>   in_d{"in_d"};
    sc_out<sc_uint<8>>  out_else{"out_else"};
    sc_out<sc_uint<8>>  out_then{"out_then"};
    sc_out<sc_uint<8>>  out_after{"out_after"};

    SC_HAS_PROCESS(A);
    explicit A(const sc_module_name& name) : sc_module(name) {
        SC_CTHREAD(callInElse, clk.pos());
        async_reset_signal_is(rst, false);
        SC_CTHREAD(callInThen, clk.pos());
        async_reset_signal_is(rst, false);
        SC_CTHREAD(callAfterIf, clk.pos());
        async_reset_signal_is(rst, false);
    }

    void callInElse() {
        sc_uint<8> d = 0;
        out_else = 0;
        wait();
        while (true) {
            const sc_uint<2> p = in_p.read();
            const sc_uint<8> m = H::mask(p);
            const sc_uint<8> x = in_d.read();
            if (p == 1) { d = 0; } else { d = H::pick(d, x, m); }
            out_else = d;
            wait();
        }
    }

    void callInThen() {
        sc_uint<8> d = 0;
        out_then = 0;
        wait();
        while (true) {
            const sc_uint<2> p = in_p.read();
            const sc_uint<8> m = H::mask(p);
            const sc_uint<8> x = in_d.read();
            if (p == 1) { d = H::pick(d, x, m); } else { d = 0; }
            out_then = d;
            wait();
        }
    }

    void callAfterIf() {
        sc_uint<8> d = 0;
        out_after = 0;
        wait();
        while (true) {
            const sc_uint<2> p = in_p.read();
            const sc_uint<8> m = H::mask(p);
            const sc_uint<8> x = in_d.read();
            if (p == 1) { d = 0; }
            d = H::pick(d, x, m);
            out_after = d;
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    sc_signal<bool> rst;
    sc_signal<sc_uint<2>> p;
    sc_signal<sc_uint<8>> d, out_else, out_then, out_after;
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    a_mod.rst(rst);
    a_mod.in_p(p);
    a_mod.in_d(d);
    a_mod.out_else(out_else);
    a_mod.out_then(out_then);
    a_mod.out_after(out_after);
    sc_start();
    return 0;
}
