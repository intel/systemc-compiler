/******************************************************************************
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// A function with two outputs through reference parameters, called twice
// in one method, one call using only one of the outputs. The assignment
// to the other output must stay in the call whose caller uses it, in
// either order of the calls. Default options: unused code is removed.
class A : public sc_module {
public:
    sc_in<sc_uint<8>>   in{"in"};
    sc_out<sc_uint<2>>  out1{"out1"};
    sc_out<sc_uint<3>>  out2{"out2"};
    sc_out<sc_uint<2>>  out3{"out3"};
    sc_out<sc_uint<3>>  out4{"out4"};

    SC_HAS_PROCESS(A);
    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(part_used_first); sensitive << in;
        SC_METHOD(part_used_second); sensitive << in;
    }

    static void split(const sc_uint<8>& x, bool& sign, sc_uint<2>& cls) {
        sign = x[7];
        cls = x == 0 ? 1 : 2;
    }

    // The call that uses only @cls comes first
    void part_used_first() {
        const sc_uint<8> x = in.read();
        bool s1 = false;
        sc_uint<2> c1 = 0;
        split(x, s1, c1);
        out1 = c1;
        bool s2 = false;
        sc_uint<2> c2 = 0;
        split(x, s2, c2);
        out2 = (sc_uint<1>(s2), c2);
    }

    // The call that uses both outputs comes first
    void part_used_second() {
        const sc_uint<8> x = in.read();
        bool s2 = false;
        sc_uint<2> c2 = 0;
        split(x, s2, c2);
        out4 = (sc_uint<1>(s2), c2);
        bool s1 = false;
        sc_uint<2> c1 = 0;
        split(x, s1, c1);
        out3 = c1;
    }
};

class B_top : public sc_module {
public:
    sc_signal<sc_uint<8>>   in{"in"};
    sc_signal<sc_uint<2>>   out1{"out1"};
    sc_signal<sc_uint<3>>   out2{"out2"};
    sc_signal<sc_uint<2>>   out3{"out3"};
    sc_signal<sc_uint<3>>   out4{"out4"};

    A a_mod{"a_mod"};

    B_top(const sc_module_name& name) : sc_module(name) {
        a_mod.in(in);
        a_mod.out1(out1);
        a_mod.out2(out2);
        a_mod.out3(out3);
        a_mod.out4(out4);
    }
};

int sc_main(int argc, char *argv[]) 
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
