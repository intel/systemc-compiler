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

// Type Converstions, SystemC_Synthesis_Subset_1_4_7: Section 6.1.2

class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<sc_uint<32> > s{"s"};
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        SC_METHOD(type_conv); sensitive << dummy;
        
    }


    // Right Shift
    template<typename T1, typename T2, typename T3>
    void type_conv_fn (T1 par1, T2 par2, T3 par3) {
        T1 A = par1;
        T2 B = par2;
        T3 C;
        B = T2(A);
        C = B;
        sct_assert_const(C==par3);
        //cout << "C = " << C << endl;
        
    }


    void type_conv()
    {
        // Signed to Unsigned:
        //    SC to SC Types: sc_int/uint/bigint/biguint
        type_conv_fn(sc_int<32>(10), sc_uint<32>(0), sc_uint<32>(10));
        type_conv_fn(sc_bigint<32>(21312), sc_biguint<32>(0), sc_biguint<32>(21312));
        //    SC Type to C++ Type
        type_conv_fn(sc_int<32>(10), unsigned(0), unsigned(10));
        type_conv_fn(sc_int<32>(10), (unsigned long)(0), (unsigned long)(10));
        type_conv_fn(sc_int<32>(10), (unsigned short)(0), (unsigned short)(10));

        //    C++ to SC Type
        type_conv_fn(int(10), sc_uint<32>(0), sc_uint<32>(10));
        type_conv_fn(long(10), sc_uint<32>(0), sc_uint<32>(10));
        type_conv_fn(short(10), sc_uint<32>(0), sc_uint<32>(10));
        type_conv_fn(int(10), sc_biguint<32>(0), sc_biguint<32>(10));
        type_conv_fn(long(10), sc_biguint<32>(0), sc_biguint<32>(10));
        type_conv_fn(short(10), sc_biguint<32>(0), sc_biguint<32>(10));

        //    C++ to C++ Type

        // Unsigned to Signed:
        //    SC to SC Types: sc_int/uint/bigint/biguint
        type_conv_fn(sc_uint<32>(10), sc_int<32>(0), sc_int<32>(10));
        type_conv_fn(sc_biguint<32>(21312), sc_bigint<32>(0), sc_bigint<32>(21312));

        //    SC Type to C++ Type
        type_conv_fn(sc_uint<32>(10), int(0), int(10));
        type_conv_fn(sc_uint<32>(10), long(0), long(10));
        type_conv_fn(sc_uint<32>(10), short(0), short(10));

        //    C++ to SC Type
        type_conv_fn(unsigned(10), sc_uint<32>(0), sc_uint<32>(10));
        type_conv_fn((unsigned long)(10), sc_uint<32>(0), sc_uint<32>(10));
        type_conv_fn((unsigned short)(10), sc_uint<32>(0), sc_uint<32>(10));
        type_conv_fn(unsigned(10), sc_biguint<32>(0), sc_biguint<32>(10));
        type_conv_fn((unsigned long)(10), sc_biguint<32>(0), sc_biguint<32>(10));
        type_conv_fn((unsigned short)(10), sc_biguint<32>(0), sc_biguint<32>(10));

        //    C++ to C++ Type
        type_conv_fn((unsigned)(10), sc_int<32>(0), sc_int<32>(10));
        type_conv_fn((unsigned long)(10), sc_int<32>(0), sc_int<32>(10));
        type_conv_fn((unsigned short)(10), sc_int<32>(0), sc_int<32>(10));
        type_conv_fn((unsigned long)(10), sc_bigint<32>(0), sc_bigint<32>(10));
        type_conv_fn((unsigned)(10), sc_bigint<32>(0), sc_bigint<32>(10));
        type_conv_fn((unsigned short)(10), sc_bigint<32>(0), sc_bigint<32>(10));

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

