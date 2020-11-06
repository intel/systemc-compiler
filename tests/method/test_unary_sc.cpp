/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"


// SC type specific unary operations
class A : public sc_module 
{
public:
    sc_signal<bool>        a{"a"};
    sc_signal<bool>        b{"b"};
    sc_signal<bool>        c{"c"};
    sc_signal<bool>*       p;
    
    int                 m;
    int                 k;
    

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        SC_METHOD(and_reduce1); sensitive << dummy;
        SC_METHOD(or_reduce1); sensitive << dummy;
        SC_METHOD(xor_reduce1); sensitive << dummy;

        SC_METHOD(sc_types_exclaim); sensitive << dummy;
        SC_METHOD(sc_types_inc_dec); sensitive << dummy;
        SC_METHOD(sc_types_comp_assign); sensitive << dummy;

        SC_METHOD(sc_to_int); sensitive << dummy;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    void and_reduce1() {
        sc_uint<4> x1 = 0xC;
        CHECK(nand_reduce(x1));
        CHECK(x1.nand_reduce());

        sc_uint<8> x2 = 0xFF;
        sc_uint<8> x3 = 0xFA;
        bool l1 = x1.and_reduce();
        bool l2 = x2.nand_reduce();
        
        CHECK(!l1);
        CHECK(!l2);
        CHECK(and_reduce(x2));
        CHECK(x3.nand_reduce());
    }     
    
    void or_reduce1() {
        sc_uint<4> x1 = 0xC;
        CHECK(!x1.nor_reduce());
        CHECK(!nor_reduce(x1));

        sc_uint<8> x2 = 0x00;
        sc_uint<8> x3 = 0x10;
        bool l1 = or_reduce(x1);
        bool l2 = x2.nor_reduce();
        
        CHECK(l1);
        CHECK(l2);
        CHECK(!x2.or_reduce());
        CHECK(!x3.nor_reduce());

        bool l3 = ((sc_uint<1>)x2).or_reduce();
        bool l4 = ((sc_uint<2>)l3).or_reduce();
        bool l5 = ((sc_uint<2>)((sc_uint<1>)x2)).or_reduce();
        l5 = ((sc_uint<1>)((sc_uint<2>)x2)).or_reduce();
        l5 = (sc_uint<1>)(((sc_uint<2>)x2).or_reduce());
        
        int i = (x1.or_reduce() || !x2.nand_reduce()) ? 1 : 2;
        i = (sc_uint<4>((sc_uint<1>)x1 + x2)).nor_reduce() && 1;
        i = 1 && ((sc_uint<1>)x1++).nor_reduce();
        i = 0 && ((sc_uint<1>)x1++).nor_reduce();
        i = 1 || x1.or_reduce();
    }     
    
     // Only 0x0 and 0x1 are considered for XOR/XNOR
     void xor_reduce1() {
        sc_uint<4> x1 = 0x1;
        sc_uint<8> x2 = 0x0;
        CHECK(x1.xor_reduce());
        CHECK(!xnor_reduce(x1));

        bool l1 = x2.xor_reduce();
        bool l2 = xnor_reduce(x2);
        
        CHECK(!l1);
        CHECK(l2);
    }     
    
    void sc_types_exclaim() {
        sc_uint<3> x = 1;
        bool b = x;
        CHECK(b);
        
        x = 0;
        CHECK(!x);
        b = 1 || !x;
        b = !x && x.or_reduce();
        
        if (!x) {
        } else {
            CHECK(0);
        }
    }
    
    void sc_types_inc_dec() {
        sc_uint<3> x = 1;
        x++;
        CHECK(x == 2);
        ++x;
        CHECK(x == 3);
        --x;
        CHECK(x == 2);
        x--;
        CHECK(x == 1);
    }
    
    void sc_types_comp_assign() {
        sc_uint<3> x = 1;
        x += 2;
        CHECK(x == 3);
        sc_int<8> y = 2;
        y -= x;
        CHECK(y == -1);
        y += x;
        CHECK(y == 2);
        y *= x;
        CHECK(y == 6);
        y = -1;
        x += y;
        CHECK(x == 2);
        x += (y+2);
        CHECK(x == 3);
        
        x += x;
        x += (!x ? y.or_reduce() : x.and_reduce()) ? 1 : 2;
    }
    
    void sc_to_int() {
        sc_biguint<66> x = 15;
        int i = x.to_int();
        CHECK(i == 15);
        x = -10;
        i = x.to_int64();
        CHECK(i == -10);
        
        x = 12;
        unsigned u = x.to_uint();
        CHECK(u == 12);
        x = 11;
        u = x.to_uint64();
        CHECK(u == 11);
        
        long unsigned ul = x.to_ulong();
        CHECK(ul == 11);
        x = -20;
        long int l = x.to_long();
        CHECK(l == -20);
    }
    
};


int sc_main(int argc, char* argv[])
{
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

