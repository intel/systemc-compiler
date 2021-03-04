/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>


// Constant evaluation for range(), concat() and bit() 
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<sc_uint<8>>   a{"a"};
    
    
    SC_CTOR(A)
    {
        SC_METHOD(bitConstProp); sensitive << a;
        SC_METHOD(rangeConstProp); sensitive << a;
        SC_METHOD(concatConstProp); sensitive << a;
        SC_METHOD(concatConstPropRHS); sensitive << a;

        SC_METHOD(constPropLHS); sensitive << a;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    void bitConstProp() 
    {
        sc_uint<4> x;
        x = 11;
        sct_assert(x.bit(3) == 1);
        x = 11;
        sct_assert_const(x.bit(3) == 1);

        x = 11;
        sct_assert(x.bit(2) == 0);
        x = 11;
        sct_assert_const(x.bit(2) == 0);

        x = 11;
        sct_assert(x[1] == 1);
        x = 11;
        sct_assert_const(x[1] == 1);

        x = 11;
        sct_assert(x[0] == 1);
        x = 11;
        sct_assert_const(x[0] == 1);

        x = 11;
        sc_uint<4> y = 5;
        sct_assert(y[1] == x[2]);
        x = 11;
        y = 5;
        sct_assert_const(y[1] == x[2]);

        x = 11;
        y = 5;
        sct_assert(y[0] == x[0]);
        x = 11;
        y = 5;
        sct_assert_const(y[0] == x[0]);
    }
    
    void rangeConstProp() 
    {
        sc_uint<4> x;
        x = 11;
        sct_assert(x.range(3,0) == 11);
        x = 11;
        sct_assert_const(x.range(3,0) == 11);

        x = 11;
        sct_assert(x.range(2,0) == 3);
        x = 11;
        sct_assert_const(x.range(2,0) == 3);

        x = 11;
        sct_assert(x.range(1,0) == 3);
        x = 11;
        sct_assert_const(x.range(1,0) == 3);

        x = 11;
        sct_assert(x.range(0,0) == 1);
        x = 11;
        sct_assert_const(x.range(0,0) == 1);

        x = 11;
        sct_assert(x(3,2) == 2);
        x = 11;
        sct_assert_const(x(3,2) == 2);

        x = 11;
        sct_assert(x(3,1) == 5);
        x = 11;
        sct_assert_const(x(3,1) == 5);

        x = 11;
        sct_assert(x(2,1) == 1);
        x = 11;
        sct_assert_const(x(2,1) == 1);

        x = 11;
        sc_uint<4> y = 5;
        sct_assert(x(3,2) == y(2,1));
        x = 11;
        y = 5;
        sct_assert_const(x(3,2) == y(2,1));
    }
    
    void concatConstProp() 
    {
        CHECK(concat((sc_uint<3>)0x3,(sc_uint<4>)0xA) == 0x3A);
        CHECK(concat(sc_uint<3>(0xA),sc_uint<4>(0x28)) == 0x28);
        CHECK(concat(sc_biguint<9>(0x1AB), sc_uint<4>(0x2)) == 0x1AB2);
        
        CHECK(concat(sc_int<4>(0x3), sc_uint<3>(0x2)) == 0x1A);
        CHECK(concat(sc_int<4>(0x3), sc_int<3>(0x2)) == 0x1A);

        CHECK( (sc_uint<4>(0xA), sc_uint<4>(0xB)) == 0xAB);
        CHECK( (sc_uint<4>(0xA), sc_uint<4>(0xB), sc_uint<4>(0xC)) == 0xABC);
        CHECK( ((sc_uint<4>(0xA), sc_uint<4>(0xB)), sc_uint<4>(0xC)) == 0xABC);
        CHECK( (sc_uint<4>(0xA), sc_uint<4>(0xB), 
                     sc_uint<4>(0xC), sc_uint<4>(0xD)) == 0xABCD);
    }

    void concatConstPropRHS() 
    {
        sc_uint<4> x;
        x = 0xB;
        
        sc_uint<5> y = 0x2;
        CHECK(concat(y,x) == 0x2B);
        CHECK(concat(x,y) == 0x162);

        sc_int<4> z = 3;
        CHECK(concat(z,x) == 0x3B);
        CHECK((z,x) == 0x3B);
        
        sc_uint<8> t = concat(y,x);
        CHECK(t == 0x2B);
        t = (y,x);
        CHECK(t == 0x2B);
        
        sc_uint<12> t2 = (x, sc_uint<4>(y), sc_uint<4>(0xA));
        CHECK(t2 == 0xB2A);
        CHECK((sc_uint<8>(0xCD), t2) == 0xCDB2A);
    }
    
    // bit() and range() in LHS, value is cleared in CPA
    void constPropLHS() 
    {
        sc_uint<4> x;
        x = 2;
        x.bit(0) = 1;
        sct_assert_unknown(x);
        
        x = 3;
        x[0] = 1;
        sct_assert_unknown(x);

        x = 4;
        x.range(2,2) = 1;
        sct_assert_unknown(x);

        x = 5;
        x(1,0) = 1;
        sct_assert_unknown(x);
    }

};

class B_top : public sc_module
{
    sc_signal<sc_uint<8>> a{"a"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

