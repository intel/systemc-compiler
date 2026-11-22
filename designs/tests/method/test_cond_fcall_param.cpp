/******************************************************************************
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Function call in a branch of conditional operator, where the function has
// no side effects but calls another function with parameters. Assigning the
// parameters of the called function is not a side effect of the caller, so no
// error is reported (issue #95).
class A : public sc_module {
public:
    sc_in<bool>         sel{"sel"};
    sc_in<sc_uint<8>>   a{"a"};
    sc_out<sc_uint<8>>  y1{"y1"};
    sc_out<sc_uint<8>>  y2{"y2"};
    sc_out<sc_uint<8>>  y3{"y3"};

    SC_HAS_PROCESS(A);
    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(static_call); sensitive << sel << a;
        SC_METHOD(member_call); sensitive << sel << a;
        SC_METHOD(local_between); sensitive << sel << a;
    }

    static sc_uint<8> inc(sc_uint<8> x) { return x + 1; }
    static sc_uint<8> twice(sc_uint<8> x) { return inc(inc(x)); }

    sc_uint<8> add(sc_uint<8> x, sc_uint<8> y) { return x + y; }
    sc_uint<8> add3(sc_uint<8> x) { return add(add(x, 1), 2); }

    sc_uint<8> add3_local(sc_uint<8> x) {
        sc_uint<8> z = add(x, 1);
        return add(z, 2);
    }

    // Static function calling a static function with a parameter
    void static_call() {
        y1 = sel.read() ? twice(a.read()) : a.read();
    }

    // Member function calling a member function with two parameters
    void member_call() {
        y2 = sel.read() ? a.read() : add3(a.read());
    }

    // Local variable between the two calls
    void local_between() {
        y3 = sel.read() ? add3_local(a.read()) : add3(a.read());
    }
};

int sc_main(int argc, char** argv) {
    sc_signal<bool> sel;
    sc_signal<sc_uint<8>> a, y1, y2, y3;
    A top("top");
    top.sel(sel);
    top.a(a);
    top.y1(y1);
    top.y2(y2);
    top.y3(y3);
    sc_start();
    return 0;
}
