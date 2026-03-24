/******************************************************************************
* Copyright (c) 2025, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_sel_type.h"
#include "sct_assert.h"
#include <iostream>

using namespace sct;
        
// Test to generate netlist and compare CPP overflow cast enabled and disabled
class A : public sc_module 
{
public:
    sc_in<unsigned> SC_NAMED(i1);
    sc_in<unsigned> SC_NAMED(i2);
    sc_in<int> SC_NAMED(si1);
    sc_in<int> SC_NAMED(si2);
    sc_in<sc_uint<64>> SC_NAMED(y1);
    sc_in<sc_uint<64>> SC_NAMED(y2);
    sc_in<sc_int<64>> SC_NAMED(sy1);
    sc_in<sc_int<64>> SC_NAMED(sy2);
    sc_in<sc_biguint<64>> SC_NAMED(x1);
    sc_in<sc_biguint<64>> SC_NAMED(x2);
    sc_in<sc_bigint<64>> SC_NAMED(sx1);
    sc_in<sc_bigint<64>> SC_NAMED(sx2);

    sc_out<sc_biguint<68>> SC_NAMED(x3);
    sc_out<sc_bigint<68>> SC_NAMED(sx3);
    sc_out<sc_biguint<68>> SC_NAMED(x4);
    sc_out<sc_bigint<68>> SC_NAMED(sx4);
    sc_out<sc_biguint<68>> SC_NAMED(x5);
    sc_out<sc_bigint<68>> SC_NAMED(sx5);
    sc_out<sc_biguint<68>> SC_NAMED(x6);
    sc_out<sc_bigint<68>> SC_NAMED(sx6);
    sc_out<sc_biguint<68>> SC_NAMED(x7);
    sc_out<sc_bigint<68>> SC_NAMED(sx7);

    SC_CTOR(A) {
        SC_METHOD(cpp_overflow);
        sensitive << i1 << i2 << si1 << si2
              << y1 << y2 << sy1 << sy2
              << x1 << x2 << sx1 << sx2;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    // C++ overflow example
    void cpp_overflow() {
        // CPP types, #330
        x3 = i1 * i2;
        sx3 = si1 * si2;
        x4 = i1 * si2;

        // SC types, #330
        x5 = y1.read() * y2.read();
        sx4 = sy1.read() * sy2.read();
        sx5 = y1.read() * sy2.read();

        // Unary 
        unsigned ti1 = i1.read();
        x6 = ++ti1;

        int tsi1 = si1.read();
        sx6 = --tsi1;

        sc_uint<64> ty1 = y1.read();
        x7 = (sc_biguint<68>)++ty1;

        sc_int<64> tsy1 = sy1.read();
        sx7 = (sc_bigint<68>)--tsy1;
    }
    
};

class B_top : public sc_module 
{
public:
    // Signals for inputs
    sc_signal<unsigned> i1{"i1"};
    sc_signal<unsigned> i2{"i2"};
    sc_signal<int> si1{"si1"};
    sc_signal<int> si2{"si2"};
    sc_signal<sc_uint<64>> y1{"y1"};
    sc_signal<sc_uint<64>> y2{"y2"};
    sc_signal<sc_int<64>> sy1{"sy1"};
    sc_signal<sc_int<64>> sy2{"sy2"};
    sc_signal<sc_biguint<64>> x1{"x1"};
    sc_signal<sc_biguint<64>> x2{"x2"};
    sc_signal<sc_bigint<64>> sx1{"sx1"};
    sc_signal<sc_bigint<64>> sx2{"sx2"};

    // Signals for outputs
    sc_signal<sc_biguint<68>> x3{"x3"};
    sc_signal<sc_bigint<68>> sx3{"sx3"};
    sc_signal<sc_biguint<68>> x4{"x4"};
    sc_signal<sc_bigint<68>> sx4{"sx4"};
    sc_signal<sc_biguint<68>> x5{"x5"};
    sc_signal<sc_bigint<68>> sx5{"sx5"};
    sc_signal<sc_biguint<68>> x6{"x6"};
    sc_signal<sc_bigint<68>> sx6{"sx6"};
    sc_signal<sc_biguint<68>> x7{"x7"};
    sc_signal<sc_bigint<68>> sx7{"sx7"};
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        // Bind inputs
        a_mod.i1(i1);
        a_mod.i2(i2);
        a_mod.si1(si1);
        a_mod.si2(si2);
        a_mod.y1(y1);
        a_mod.y2(y2);
        a_mod.sy1(sy1);
        a_mod.sy2(sy2);
        a_mod.x1(x1);
        a_mod.x2(x2);
        a_mod.sx1(sx1);
        a_mod.sx2(sx2);

        // Bind outputs
        a_mod.x3(x3);
        a_mod.sx3(sx3);
        a_mod.x4(x4);
        a_mod.sx4(sx4);
        a_mod.x5(x5);
        a_mod.sx5(sx5);
        a_mod.x6(x6);
        a_mod.sx6(sx6);
        a_mod.x7(x7);
        a_mod.sx7(sx7);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

