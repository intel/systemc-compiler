/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_sel_type.h"
#include "sct_assert.h"
#include <iostream>

// Shifts of negative literals casted to C++ and SC signed types
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<sc_uint<32> > s{"s"};
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        SC_METHOD(shift_right_neg); sensitive << dummy;
        
        SC_METHOD(shift_left_neg); sensitive << dummy;
    }

    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    // Right Shift
    template<typename T1, typename T2, typename T3>
    void shift_right_neg_fns (T1 par1, T2 par2, T3 par3) {
        T1 A = par1;
        T2 B = par2;
        T3 C = par3;
        
        sc_bigint<64> shift_r1 = A >> B;
        CHECK(shift_r1==C);
        // SV: assert (shift_r1 == C) else $error("Assertion failed at test_shifts_signed.cpp:33:9 shift_r1: %d and C: %d", shift_r1, C);
        cout << "A >> B = " << shift_r1 << endl;
        
    }

    template<typename T1, typename T2, typename T3>
    void shift_left_neg_fns (T1 par1, T2 par2, T3 par3) {
        T1 A = par1;
        T2 B = par2;
        T3 C = par3;

        sc_bigint<64> shift_l1 = A << B;
        CHECK(shift_l1==C);
        cout << "A << B = " << shift_l1 << endl;

    }

    void shift_right_neg()
    {
    	shift_right_neg_fns(sc_int<32>(-10), sc_int<32>(31), sc_bigint<64>(-1));
    	shift_right_neg_fns(sc_int<32>(-0), sc_int<32>(33), sc_bigint<64>(0));
    	shift_right_neg_fns(sc_int<32>(-1), sc_int<32>(10), sc_bigint<64>(-1));
    	shift_right_neg_fns(sc_int<32>(-2147483648), sc_int<32>(31), sc_bigint<64>(-1));
    	shift_right_neg_fns(sc_int<20>(-55), sc_int<32>(0), sc_bigint<64>(-55));
    	shift_right_neg_fns(sc_int<32>(0), sc_int<32>(0), sc_bigint<64>(0));
    	shift_right_neg_fns(sc_bigint<63>(-10), sc_bigint<64>(31), sc_bigint<64>(-1));



    	//shift_right_neg_fns(sc_int<32>(-10), sc_int<32>(33), sc_bigint<64>(-1));
    	// -10  : 1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_0110
    	//  10  :                                                                            1010
    	// ~(10): 1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_0101
    	// ~10+1: 1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_0110
    	// >> 31:                                       1_1111_1111_1111_1111_1111_1111_1111_1111 (8589934591)

    }
    void shift_left_neg()
    {
    	shift_left_neg_fns(sc_int<32>(-10), sc_int<32>(4), sc_bigint<64>(-160)); //8589934591));
    	// SV: assert (shift_l1 == C) else $error("Assertion failed at test_shifts_signed.cpp:45:9 shift_l1: %d and C: %d", shift_l1, C);
        shift_left_neg_fns(sc_int<32>(-30), sc_int<32>(8), sc_bigint<64>(-7680));
        shift_left_neg_fns(sc_int<32>(-0), sc_int<32>(33), sc_bigint<64>(0));
        shift_left_neg_fns(sc_int<32>(-1), sc_int<32>(10), sc_bigint<64>(-1024));
        shift_left_neg_fns(sc_int<32>(-2147483648), sc_int<32>(31), sc_bigint<64>(-4611686018427387904));
        shift_left_neg_fns(sc_int<20>(-55), sc_int<32>(0), sc_bigint<64>(-55));
        shift_left_neg_fns(sc_int<32>(0), sc_int<32>(0), sc_bigint<64>(0));
        shift_left_neg_fns(sc_bigint<32>(-1), sc_bigint<32>(10), sc_bigint<64>(-1024));
                	// -10  : 1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_0110
        	//  10  :                                                                            1010
        	// ~(10): 1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_0101
        	// ~10+1: 1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_0110
        	// << 4:  1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_0110_0000

    }
};

class B_top : public sc_module 
{
public:
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

