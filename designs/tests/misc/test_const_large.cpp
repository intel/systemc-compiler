/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>
#include <iostream>

// Large constants to check correct localparam initialization value generated
struct A : public sc_module 
{
    sc_in<bool> clk{"clk"};
    sc_signal<sc_uint<32>> s;

    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(constMeth);
        sensitive << s;
    }

    // Large constants should be HEX
    const uint64_t BIG_CONST1  = (~0ULL);
    static const uint64_t BIG_CONST2  = (~0ULL);
    static const unsigned long BIG_CONST3  = (~0ULL);
    //static const long BIG_CONST4  = -(1UL << 63);  // Overflow, ICSCS reports error
    static const long BIG_CONST5  = -8;
    static const unsigned long BIG_CONST6  = (1ULL << 63);
    static const unsigned long BIG_CONST7  = 47;

    static const uint64_t INT_CONST1 = 2147483647;
    static const unsigned INT_CONST2 = 2147483648;
    static const int64_t INT_CONST3 = -2147483647;
    static const long INT_CONST4 = -2147483648;
    static const long INT_CONST5 = -2;
    static const int64_t INT_CONST6 = -0x40000000;
    static const int64_t INT_CONST7 = -1073741824;
    static const int64_t INT_CONST8 = -0x3FFFFFFF;
    static const int64_t INT_CONST9 = -1073741823;

    void constMeth() 
    {
        auto a = BIG_CONST1 + BIG_CONST2 + BIG_CONST3 /*+ BIG_CONST4*/ +
                 INT_CONST1 + INT_CONST2 + INT_CONST3 + INT_CONST4 + 
                 INT_CONST6 + INT_CONST7 + INT_CONST8 + INT_CONST9; 
        int64_t b = 10000000000;
        b = -10000000000;
        b = a + 10000000000;
        auto c = /*BIG_CONST4 +*/ BIG_CONST5 + BIG_CONST6 + BIG_CONST7;
        int i = -0X100 - 100 /*+ BIG_CONST4*/;
        long d = INT_CONST3;
        d = INT_CONST5;
        d = -2147483647;
        d = -2147483648;
        d = -2;
    }
    
};

struct Top : public sc_module 
{
    sc_in<bool>     clk{"clk"};
    A               modA{"modA"};

    SC_CTOR(Top)  
    {
        modA.clk(clk);
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    Top mod{"mod"};
    mod.clk(clock_gen);
    sc_start();

    return 0;
}

