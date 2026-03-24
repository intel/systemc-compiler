/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
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
    const sc_bigint<65> by = 42;
    const sc_bigint<65> by1 = -42;

    const sc_bigint<67> sy1 = "-36893488147419103231";   // -0x1F...F
    const sc_bigint<67> sy2 = "-55340232221128654847";   // -0x2F...F
    const sc_bigint<68> sy3 = "-73786976294838206463";   // -0x3F...F 
    const sc_bigint<68> sy4 = -sct_ones<65>;
    const sc_bigint<68> sy5 = -sct_ones<66>;
    
    // Maximal width specified with @LITERAL_MAX_BIT_NUM
    const sc_biguint<16384> very_large = 42;

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
        const_cast<sc_uint<16>&>(x) = i << 2;    // 48
        const_cast<sc_bigint<64>&>(y) = i - x;   // -36
        
        for (int i = 0; i < N; i++) {
            const_cast<unsigned&>(arr[i]) = i+1;
        }

        j = 10;
        z = 11;
        
        SC_METHOD(useLargeConst); sensitive << s;
        SC_METHOD(cpaLargeConst); sensitive << s;
    }
    
    const sc_uint<64> AA = 0x9000000000000008;
    const sc_uint<64> BB = 0x7000000000000006;
    
    sc_biguint<128> M0;
    sc_biguint<128> M1 = (AA, BB);              
    sc_biguint<128> M2 = 0xF000000000000000;     
    sc_biguint<128> M3 = "0x0F000000000000000";        // Lead 0x0 required!!!
    
    sc_biguint<66> Z0 = sct_zeros<66>;
    const sc_biguint<67> Z1 = sct_zeros<67>;
    const sc_biguint<67> Z2 = 0;
    const sc_uint<27> Z3 = 0;

    const sc_biguint<128> C1 = (AA, BB);        
    const sc_biguint<128> C2 = 0xF000000000000000;  
    const sc_biguint<128> C3 = "0x0F000000000000000";  // Lead 0x0 required!!!
    const sc_biguint<100> C4 = "0x0F0000000000000000";  // Lead 0x0 required!!!

    static const sc_biguint<128> S1;
    
    int I1 = 42;
    const int I2 = 43;
    
    const sc_bv<65> bv1 = 242;
    const sc_bv<65> bv2 = "0x0F2";
    const sc_bv<65> bv3 = 0xFFFFFFFFFFFFFFFF;
    const sc_bv<65> bv4 = sct_ones<65>;
    const sc_bv<66> bv5 = "0x2FFFFFFFFFFFFFFFF";
    
    // Use >64bit constant
    sc_signal<sc_biguint<128>> t0;
    void useLargeConst()
    {
        sc_uint<27> z3 = 0; 
        M0 = (AA, BB);                // OK
        M0 = 0xF000000000000000;      // OK
        M0 = "0xFF000000000000000";   // OK
        t0 = M0 + M1 + M2 + M3;
        t0 = C1 + C2 + C3;
        t0 = I1 + I2;
        t0 = by + by1 + uy + x +y;
        t0 = sy1 + sy2 + sy3 + sy4 + sy5; 
        t0 = Z0;
        t0 = Z1;
        t0 = Z2;
        t0 = (sc_biguint<128>)Z3 + (sc_biguint<128>)z3;
        
        sc_biguint<16384> vl = very_large;
        t0 = vl;
        
        t0 = bv1.to_int() + bv2.to_int() + bv3.to_int() + bv4.to_int() + bv5.to_int();
    }
    
    // Constant propagation for >64bit
    sc_signal<sc_biguint<128>> t1;
    void cpaLargeConst()
    {
        sct_assert_const(C1 != 0);
        sct_assert_const(C2 == C3);
        sct_assert_const(S1 == C4);
        sct_assert_const(C4.bit(67) == 1);
        sct_assert_const(C4 >> 64 == 0xF);
        sct_assert_const(((C4 >> 64) & 0xA) == 0xA);
        sc_biguint<128> l = C4;
        
        if (l == 0) {
            t1 = 1;
        }
        t1 = 0;
    }
    
};

const sc_biguint<128> A::S1 = "0x0F0000000000000000";  


int sc_main(int argc, char **argv) {

    A modA{"modA", 1, 11};
    sc_start();

    return 0;
}

