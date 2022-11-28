/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>
#include <stdint.h>
#include <iostream>
#include <string>

using namespace sc_core;

// Constant evaluation in binary statements
class A_mod : public sc_module {
public:
    
    SC_HAS_PROCESS(A_mod);
    const unsigned A = 1;
    static const unsigned B = 2;
    static const unsigned Z = 0;
    const int C = -1;
    static const long D = -2;
    
    sc_signal<sc_uint<4>> s{"s"};
    
    A_mod(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(ref_const); sensitive << s;
        
        SC_METHOD(shorten_const); sensitive << s;
        SC_METHOD(big_shift); sensitive << s;
        
        SC_METHOD(concat_const); sensitive << s;
        SC_METHOD(concat_local_const); sensitive << s;
        
        SC_METHOD(concat_const_var); sensitive << s;
      
        SC_METHOD(concat_const_cast); sensitive << s;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    void big_shift() {
        sc_biguint<131> a = (sc_biguint<131>)1 << 130;
        sc_biguint<142> b = (a + 3) << 10;
        sc_biguint<139> c = b - a + 1;
        sc_uint<20> d = c >> 120;
        
        CHECK(d == 0x07fc00);
        
        /*
        std::cout << hex;
        a.print(std::cout); std::cout << std::endl;
        b.print(std::cout); std::cout << std::endl;
        c.print(std::cout); std::cout << std::endl;
        d.print(std::cout); std::cout << std::endl;*/
    }    
    
    // Constant and constant
    const sc_uint<8> K = 0x11;
    const sc_int<8> L = 0x11;
    const sc_int<8> M = -42;
    const sc_biguint<64> N = 0x555;
    const sc_bigint<64> P = -0x555;

    const sc_uint<4> CK = 0x32;
    const sc_biguint<8> CL = 0x456;
    
    // Reference constant
    void ref_const() 
    {
        sc_uint<32> z;
        const sc_uint<4>& RK = 0x34;
        cout << "RK " << RK << endl;
        
        // Constant reference not replaced with value
        z = (K, RK);
        cout << hex << "z " << z << dec << endl;
        //CHECK(z == 0x114);
    }

    // Shorten constant 
    void shorten_const() 
    {
        sc_uint<32> z = CK + 1;
        CHECK(z == 3);
        
        z = (K, CK);
        CHECK(z == 0x112);

        z = (CK, CL);
        CHECK(z == 0x256);
        
        z = (CK, sc_uint<4>(5));
        CHECK(z == 0x25);
        
        const sc_uint<4> CKL = 0x78;
        z = (CK, CKL);
        CHECK(z == 0x28);
    }
    
    void concat_const() 
    {
        sc_uint<4> a = ((sc_uint<2>)1, sc_uint<2>(2));
        CHECK(a == 6);
        
        a = ((sc_uint<2>)(A+1), sc_uint<2>(B));
        CHECK(a == 10);

        // Narrowing and widening
        a = ((sc_uint<3>)1, sc_uint<1>(2));
        CHECK(a == 2);
        a = ((sc_uint<3>)(A), sc_uint<1>(B));
        CHECK(a == 2);
        
        sc_uint<16> b = (K, L);
        CHECK(b == 0x1111);
        
        b = (K, (sc_uint<4>)L);
        CHECK(b == 0x111);
        
        b = (sc_uint<4>(K), L);
        std::cout << hex << b << std::endl;
        CHECK(b == 0x111);

        b = (sc_uint<2>(K), sc_uint<10>(L));
        std::cout << hex << b << std::endl;
        CHECK(b == 0x411);

        sc_biguint<72> d = (N, K);
        std::cout << hex << d << std::endl;
        CHECK(d == 0x55511);
    }
    
    // Local constant and constant concat
    void concat_local_const() 
    {
        const sc_uint<8> KL = 0x11;
        const sc_biguint<12> KM = 0x222;

        sc_uint<32> b = (sc_uint<2>(1), KL);
        CHECK(b == 0x111);
        
        b = (sc_uint<5>(4), KL, sc_uint<4>(7));
        std::cout << hex << b << std::endl;
        CHECK(b == 0x4117);
        
        b = (L, KL);
        std::cout << hex << b << std::endl;
        CHECK(b == 0x1111);
        
        b = (sc_uint<3>(K), sc_uint<5>(KL));
        CHECK(b == 0x31);

        sc_biguint<72> d = (KL, KM);
        std::cout << hex << d << std::endl;
        CHECK(d == 0x11222);

        d = (N, KM, K);
        std::cout << hex << d << std::endl;
        CHECK(d == 0x55522211);
    }

    // Constant and variable
    void concat_const_var() 
    {
        sc_uint<4> a = s.read();
        sc_uint<8> b = ((sc_uint<2>)1, a);
        b = (a, (sc_uint<4>)0xFF);
        b = (a, (sc_uint<4>)(254 + 1));
        sc_uint<16> c = (b, K);
        c = (K, a);
        
        sc_uint<16> d = (sc_uint<11>)K;
        d = (sc_uint<11>)a;
        d = ((sc_uint<11>)K, (sc_uint<3>)a);
        d = ((sc_uint<3>)K, (sc_uint<5>)a);
        
        sc_uint<20> e = (sc_uint<11>)(K + a*B);
        e = (sc_uint<11>)((sc_uint<3>)K + (B >> 2));
        e = (sc_uint<7>)((sc_uint<7>)a + sc_uint<14>(K));
    }
    
    // Constant with multiple casts
    void concat_const_cast() 
    {
        sc_uint<16> a = ((sc_uint<8>)((sc_uint<4>)0x55), 
                        (sc_uint<8>)((sc_uint<4>)0x11));
        std::cout << hex << a << std::endl;
        CHECK(a == 0x501);
        
        a = ((sc_uint<1>)1, (sc_uint<6>)((sc_uint<4>)(K+1)), 
             (sc_uint<6>)(sc_uint<4>(L)));
        std::cout << hex << a << std::endl;
        CHECK(a == 0x1081);
        
        sc_uint<7> b = (sc_uint<8>)N;
        std::cout << hex << b << std::endl;
        CHECK(b == 0x55);
        
        b = (sc_uint<6>)N;
        std::cout << hex << b << std::endl;
        CHECK(b == 0x15);
    }
};


int sc_main(int argc, char *argv[]) {
    A_mod a_mod{"a_mod"};
    sc_start();
    return 0;
}

