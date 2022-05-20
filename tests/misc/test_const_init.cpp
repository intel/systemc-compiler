/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Constant initialization in constructor body
struct A : public sc_module 
{
    sc_signal<sc_uint<32>> s;
    
    const bool b = 0;
    const int  i = 0;
    const sc_uint<16> x = 0;
    const sc_bigint<64> y = 0;
    const sc_biguint<64> uy = 1ULL << 63;
    //const sc_bigint<65> by = 42; -- error reported
    
    const sc_uint<64> AA = 0x8000000000000000;
    const sc_uint<64> BB = 0x8000000000000000;
    //const sc_biguint<128> C = (AA, BB);  -- error reported
    
    static const size_t N = 3;
    const unsigned arr[N] = {0,0,0};

    // Non-constant fields have no initialization value after elaboration
    int j;
    sc_uint<16> z;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name, bool b_, int i_) : 
        sc_module(name) 
    {
        const_cast<bool&>(b) = b_;
        const_cast<int&>(i)  = i_+1;
        const_cast<sc_uint<16>&>(x) = i << 2;
        const_cast<sc_bigint<64>&>(y) = i - x;
        
        for (int i = 0; i < N; i++) {
            const_cast<unsigned&>(arr[i]) = i+1;
        }

        j = 10;
        z = 11;
        
        SC_METHOD(constInit);
        sensitive << s;

        SC_METHOD(unknownConstInit);
        sensitive << s;
    
        SC_METHOD(unknownConstFunc);
        sensitive << s;
    }
    
    void constInit() 
    {
        sct_assert_const(b);
        sct_assert_const(i == 12);
        sct_assert_const(x == 48);
        sct_assert_const(y == -36);
        sct_assert_const(uy == 1ULL << 63);

        if (b) {
            s = i + x + j + z;
        }
        
        sc_biguint<64> bu = uy + AA + BB;

        for (int i = 0; i < N; i++) {
            sct_assert_const(arr[i] == i+1);
        }
    }
    
    void unknownConstInit() 
    {
        const bool c1 = s.read() == 42;
        const int c2  = s.read();
        const sc_uint<16> c3 = sc_uint<16>(s.read());
        const sc_bigint<65> c4 = sc_bigint<65>(s.read());
        
        sct_assert_unknown(c1);
        sct_assert_unknown(c2);
        sct_assert_unknown(c3);
        sct_assert_unknown(c4);
        
        const bool d1 = c2 == c3;
        const unsigned d2 = d1 ? c3.to_uint() : c4.to_uint();
        const sc_uint<20> d3 = sc_uint<20>(d2);
        const sc_biguint<100> d4 = sc_biguint<100>(c4+1);
        
        sct_assert_unknown(d1);
        sct_assert_unknown(d2);
        sct_assert_unknown(d3);
        sct_assert_unknown(d4);
    }

    
    int f1() {
        return s.read();
    }
    
    sc_biguint<80> f2(int i) {
        return (sc_biguint<80>(f1()));
    }

    sc_int<22> f3(int i) {
        return (sc_int<22>(i+1));
    }

    template<typename T>
    sc_uint<16> f4(T i) {
        return (sc_uint<16>)i.range(15,0);
    }

    void unknownConstFunc() 
    {
        const int e1 = f1();
        const auto e2  = f2(e1);
        const auto e3 = f3(s.read());
        const auto e4 = f4(e3);
        
        sct_assert_unknown(e1);
        sct_assert_unknown(e2);
        sct_assert_unknown(e3);
        sct_assert_unknown(e4);        
    }
};


int sc_main(int argc, char **argv) {

    A modA{"modA", 1, 11};
    sc_start();

    return 0;
}

