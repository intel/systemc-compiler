/******************************************************************************
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Explicit cast to unsigned type of signed non-literal is ignored in SV: its
// result keeps the sign, so it is sign extended, while in SC it is zero
// extended. That differs if the cast result high bit is set, warning reported
// where the result is extended: wider cast, assignment, initialization,
// function argument and return. Not reported for operand signed in C++ only,
// like unsigned value converted with to_long(), which is unsigned in SV.
// Lines with the warning are marked "warning",
// method_cast_unsigned_extend_WARN checks it is reported there and only there.
class A : public sc_module {
public:
    sc_in<sc_int<11>>   e{"e"};
    sc_in<int>          i{"i"};
    sc_in<sc_uint<10>>  u{"u"};

    sc_out<sc_uint<16>> out_cast{"out_cast"};
    sc_out<sc_int<16>>  out_scast{"out_scast"};
    sc_out<sc_uint<16>> out_static{"out_static"};
    sc_out<sc_uint<16>> out_port{"out_port"};
    sc_out<sc_uint<16>> out_var{"out_var"};
    sc_out<sc_uint<16>> out_decl{"out_decl"};
    sc_out<sc_uint<32>> out_chain{"out_chain"};
    sc_out<sc_uint<64>> out_ulong{"out_ulong"};
    sc_out<sc_uint<64>> out_touint{"out_touint"};
    sc_out<sc_uint<16>> out_arg{"out_arg"};
    sc_out<sc_uint<16>> out_ret{"out_ret"};

    sc_out<sc_uint<16>> out_concat{"out_concat"};
    sc_out<sc_uint<10>> out_same{"out_same"};
    sc_out<sc_uint<8>>  out_narrow{"out_narrow"};
    sc_out<bool>        out_cmp{"out_cmp"};
    sc_out<sc_uint<16>> out_uns{"out_uns"};
    sc_out<sc_uint<48>> out_impl{"out_impl"};
    sc_out<bool>        out_cond{"out_cond"};
    sc_out<sc_uint<16>> out_liter{"out_liter"};
    sc_out<sc_uint<16>> out_tolong{"out_tolong"};

    SC_HAS_PROCESS(A);
    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(extended); sensitive << e << i;
        SC_METHOD(not_extended); sensitive << e << i << u;
    }

    sc_uint<16> pass(sc_uint<16> x) {
        return x;
    }

    sc_uint<16> low(const sc_int<11>& x) {
        return sc_uint<10>(x);                              // warning
    }

    void extended() {
        out_cast = sc_uint<16>(sc_uint<10>(e.read()));     // warning
        out_scast = sc_int<16>(sc_uint<10>(e.read()));     // warning
        out_static = static_cast<sc_uint<10>>(e.read());   // warning
        out_port = sc_uint<10>(e.read());                  // warning

        sc_uint<16> a;
        a = sc_uint<10>(e.read());                         // warning
        out_var = a;

        sc_uint<16> b = sc_uint<10>(e.read());             // warning
        out_decl = b;

        // Reported once, at the unsigned cast
        sc_uint<32> c = sc_uint<16>(sc_uint<10>(e.read()));// warning
        out_chain = c;

        unsigned long d = 0;
        d = (unsigned)i.read();                            // warning
        out_ulong = d;

        sc_uint<64> f = e.read().to_uint();                // warning
        out_touint = f;

        out_arg = pass(sc_uint<10>(e.read()));             // warning
        out_ret = low(e.read());
    }

    void not_extended() {
        // Not extended
        out_concat = (sc_uint<6>(0), sc_uint<10>(e.read()));
        sc_uint<10> g;
        g = sc_uint<10>(e.read());
        out_same = g;
        out_narrow = sc_uint<10>(e.read());
        out_cmp = sc_uint<10>(e.read()) > 500;
        out_cond = sc_uint<42>(i.read()) != 0;

        // Unsigned operand, no explicit cast, literal
        out_uns = sc_uint<16>(sc_uint<10>(u.read()));
        sc_uint<48> h = i.read();
        out_impl = h;
        out_liter = sc_uint<16>(sc_uint<10>(5));
        
        // Operand signed in C++ only
        out_tolong = sc_uint<8>(u.read().to_long());
    }
};

class B_top : public sc_module {
public:
    sc_signal<sc_int<11>>   e{"e"};
    sc_signal<int>          i{"i"};
    sc_signal<sc_uint<10>>  u{"u"};

    sc_signal<sc_uint<16>>  out_cast{"out_cast"};
    sc_signal<sc_int<16>>   out_scast{"out_scast"};
    sc_signal<sc_uint<16>>  out_static{"out_static"};
    sc_signal<sc_uint<16>>  out_port{"out_port"};
    sc_signal<sc_uint<16>>  out_var{"out_var"};
    sc_signal<sc_uint<16>>  out_decl{"out_decl"};
    sc_signal<sc_uint<32>>  out_chain{"out_chain"};
    sc_signal<sc_uint<64>>  out_ulong{"out_ulong"};
    sc_signal<sc_uint<64>>  out_touint{"out_touint"};
    sc_signal<sc_uint<16>>  out_arg{"out_arg"};
    sc_signal<sc_uint<16>>  out_ret{"out_ret"};

    sc_signal<sc_uint<16>>  out_concat{"out_concat"};
    sc_signal<sc_uint<10>>  out_same{"out_same"};
    sc_signal<sc_uint<8>>   out_narrow{"out_narrow"};
    sc_signal<bool>         out_cmp{"out_cmp"};
    sc_signal<sc_uint<16>>  out_uns{"out_uns"};
    sc_signal<sc_uint<48>>  out_impl{"out_impl"};
    sc_signal<bool>         out_cond{"out_cond"};
    sc_signal<sc_uint<16>>  out_liter{"out_liter"};
    sc_signal<sc_uint<16>>  out_tolong{"out_tolong"};

    A a_mod{"a_mod"};

    B_top(const sc_module_name& name) : sc_module(name) {
        a_mod.e(e); a_mod.i(i); a_mod.u(u);
        a_mod.out_cast(out_cast); a_mod.out_scast(out_scast);
        a_mod.out_static(out_static); a_mod.out_port(out_port);
        a_mod.out_var(out_var); a_mod.out_decl(out_decl);
        a_mod.out_chain(out_chain); a_mod.out_ulong(out_ulong);
        a_mod.out_touint(out_touint); a_mod.out_arg(out_arg);
        a_mod.out_ret(out_ret);
        a_mod.out_concat(out_concat); a_mod.out_same(out_same);
        a_mod.out_narrow(out_narrow); a_mod.out_cmp(out_cmp);
        a_mod.out_uns(out_uns); a_mod.out_impl(out_impl);
        a_mod.out_cond(out_cond); a_mod.out_liter(out_liter);
        a_mod.out_tolong(out_tolong);
    }
};

int sc_main(int argc, char *argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
