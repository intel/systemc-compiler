/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// @auto type for declaration with initialization of @sc_bigint/@sc_biguint and 
// @sc_signed/@sc_unsigned
class A : public sc_module 
{
public:
    sc_signal<bool>         bsig;
    sc_signal<int>          isig;
    sc_signal<unsigned>     usig;
    sc_signal<sc_int<8>>    scisig;
    sc_signal<sc_uint<8>>   scusig;

    sc_signal<sc_bigint<66>> int_sig;
    sc_signal<sc_biguint<66>> uint_sig;

    SC_CTOR(A)
    {
        SC_METHOD(cpp_types_to_auto); sensitive << bsig << isig << usig;
        SC_METHOD(sc_types_to_auto); sensitive << scisig << scusig;
        SC_METHOD(sc_signed_to_auto); sensitive << int_sig;
        SC_METHOD(sc_unsigned_to_auto); sensitive << uint_sig; 
    }
    
    void cpp_types_to_auto() 
    {
        sc_int<16> a;
        sc_uint<16> c;
        auto b1 = bsig.read();
        auto b2 = bsig.read() ^ 1;  // integer
        auto b3 = isig.read();
        auto b4 = isig.read() * 3;
        auto b5 = isig.read() - a;  // long
        auto b6 = (sc_int<7>)isig.read();

        auto b7 = usig.read();
        auto b8 = usig.read() + 2U;
        auto b9 = usig.read() + c;  // unsigned long
        auto b10 = (sc_uint<7>)(usig.read());
    }
    
    void sc_types_to_auto() 
    {
        auto b1 = scisig.read();
        auto b2 = (sc_int<16>)scisig.read();
        auto b3 = (sc_uint<16>)scisig.read();

        auto b4 = scusig.read();
        auto b5 = (sc_int<16>)scusig.read();
        auto b6 = (sc_uint<16>)scusig.read();
    }
    
    void sc_signed_to_auto() 
    {
        sc_bigint<66> a;
        auto b1 = int_sig.read();
        auto b2 = int_sig.read() + 1;
        auto b3 = a;
        auto b4 = a + 1;
        auto b5 = sc_bigint<67>(a + 1);
        auto b6 = sc_biguint<67>(a + 1);
    }
    
    void sc_unsigned_to_auto() 
    {
        sc_biguint<66> a;
        auto b1 = uint_sig.read();
        auto b2 = uint_sig.read() + 1;      // Result is signed
        auto b2a = uint_sig.read() + 1U;    // Result is unsigned
        auto b3 = a;
        auto b4 = a >> 2;
        auto b5 = sc_biguint<67>(a >> 2);
        auto b6 = sc_bigint<67>(a >> 2);
    }
};

int sc_main(int argc, char* argv[])
{
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

