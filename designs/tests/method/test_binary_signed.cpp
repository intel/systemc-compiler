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

// Operations with negative numbers of C++ and SC types, 
// mixed of signed and unsigned types -- should not be normally used
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<sc_uint<32> > s{"s"};
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(github_16); sensitive << sa << sb;
        
        SC_METHOD(liter_suff_U); sensitive << dummy;
        SC_METHOD(liter_suff_L); sensitive << dummy;
        SC_METHOD(liter_suff_var); sensitive << dummy;
        
        SC_METHOD(liter_neg_non_div_bug); sensitive << dummy;
        
        SC_METHOD(liter_neg_div); sensitive << dummy;
        SC_METHOD(liter_neg_div_part); sensitive << dummy;
        SC_METHOD(liter_neg_non_div); sensitive << dummy;
        
        SC_METHOD(cpp_type_neg_div); sensitive << dummy;
        SC_METHOD(sc_type_neg_div); sensitive << dummy;
        SC_METHOD(sc_bigint_type_neg_div); sensitive << dummy;
        SC_METHOD(sc_biguint_type_neg_div); sensitive << dummy;
        SC_METHOD(sc_type_pos_div_rem); sensitive << dummy;
        SC_METHOD(sc_type_neg_div2); sensitive << dummy;
        SC_METHOD(expl_sign_cast); sensitive << dummy;
        SC_METHOD(sc_types_64bit); sensitive << dummy;
        
        SC_METHOD(sc_type_neg_comp); sensitive << dummy;
        SC_METHOD(cpp_sc_type_mix); sensitive << dummy;
        SC_METHOD(compl_expr_mix); sensitive << dummy;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    sc_signal<uint16_t> sa;
    sc_signal<uint16_t> sb;

    void github_16() {
        uint16_t ua;
        uint16_t ub;
        sc_uint<16> usa;
        sc_uint<16> usb;
        uint16_t result;
        sc_uint<16> us_result;

        // signed` in SV 
        result = ua - ub;
        result = sa.read() - sb.read();
        // No signed` in SV
        us_result = usa - usb;
    }
    
    // Literal suffix @U just ignored
    void liter_suff_U() 
    {
        sc_int<3> x = 2;
        sc_int<4> y = -6;
        sc_int<8> z;
        z = 6 / 2U;
        CHECK(z == 3);
        z = (-6) / 2U;      // Warning repotred
        //CHECK(z == -3);

        z = 12U / 6;
        CHECK(z == 2);
        z = 12U / (-6);     // Warning repotred
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 0);  -2 in SV
    }
    
    // Literal suffix @L just ignored
    void liter_suff_L() 
    {
        sc_int<3> x = 2;
        sc_int<4> y = -6;
        sc_int<8> z;
        sc_bigint<70> bz;
        z = 6 / 2L;
        CHECK(z == 3);
        z = (-6) / 2L;
        CHECK(z == -3);

        z = 12L / 6;
        CHECK(z == 2);
        z = 12L / (-6);
        std::cout << "z = " << z << std::endl;
        CHECK(z == -2);
        
        bz = (1LL << 62);
        std::cout << "bz = " << bz << std::endl;
        CHECK(bz == 4611686018427387904LL);
        bz = (1LL << 62) * (-1);
        std::cout << "bz = " << bz << std::endl;
        CHECK(bz == -4611686018427387904LL);
        bz = (1LL << 62) * (-1) / 2;
        std::cout << "bz = " << bz << std::endl;
        CHECK(bz == -2305843009213693952LL);
        
        bz = 4611686018427387900LL + 4;
        CHECK(bz == 4611686018427387904LL);
        bz = -4611686018427387900LL - 4;
        CHECK(bz == -4611686018427387904LL);
        bz = -4611686018427387900LL / 2;
        CHECK(bz == -2305843009213693950LL);

        bz = 4611686018427387900ULL / 2;
        CHECK(bz == 2305843009213693950ULL);
    }
    
    // Mix of literals with suffix and variables
    void liter_suff_var() 
    {
        int i = -6;
        sc_int<4> x = 6;
        sc_int<5> y = -6;
        sc_int<8> z = -6;
        sc_bigint<70> bz;
        
        bz = x / 2L;
        std::cout << "bz = " << bz << std::endl;
        CHECK(bz == 3);
        bz = y / 2L;
        CHECK(bz == -3);
        bz = i / 2L;
        CHECK(bz == -3);
        bz = z / 2L;
        CHECK(bz == -3);
        
        bz = 12L / x;
        CHECK(bz == 2);
        bz = 12L / y;
        CHECK(bz == -2);
        bz = 12L / i;
        CHECK(bz == -2);
        bz = 12L / z;
        CHECK(bz == -2);
    }

//---------------------------------------------------------------------------
    
    void liter_neg_non_div_bug() 
    {
        int i; 
        int j = 4;
        sc_int<16> z;
        sc_biguint<16> bu = 1;
        sc_bigint<16> bz = -5;
        sc_int<8> x = -5;
        sc_uint<4> ux = 2;
        unsigned uu = 42;

        z = bu + (ux + x);      // Warning repotred
        std::cout << "z = " << z << std::endl;
        //CHECK(z == -2); 
        
        i = -1;
        i = j + uu * i;         // Warning repotred
        //CHECK(i == -38); 
        
        z = 1;
        bz = (bu * bz - x);
        
        z = -6 / ux;            // Warning repotred
        bz = (-6) + ux * (-3);
        //CHECK(bz == -12);  
        
        // Unsigned division, incorrect expression
        z = (-uu*4) / (-2);     // Warning repotred
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 0); 
        
        // Literal expression
        uu = ux + (1 + 2);      // Warning repotred
        uu = ux / (1 << 2);     // Warning repotred
    }
    
    void liter_neg_div() 
    {
        sc_int<3> x = 2;
        sc_int<4> y = -6;
        sc_int<8> z;
        sc_uint<4> ux = 2;
        
        z = (-6) / 2;
        CHECK(z == -3);
        z = (-6) * 2 - 3;
        CHECK(z == -15);
        z =  12 /(-4) + 3 * (-1);
        CHECK(z == -6);
        z = 2 / (5 - 6);
        CHECK(z == -2);
        z = 2 / (5 + (-6));
        CHECK(z == -2);
        
        z = sc_int<4>(-6) / sc_int<4>(2);
        CHECK(z == -3);
        z = sc_int<4>(-6) * sc_int<4>(2) + sc_bigint<5>(3);
        CHECK(z == -9);
        z = 2 / (sc_int<4>(5) - sc_int<4>(6));
        CHECK(z == -2);
        z = 2 / (sc_int<4>(5) + (-sc_int<4>(6)));
        CHECK(z == -2);
        
        unsigned uu = 2;
        z = -6 / uu;            // Warning repotred
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);

        // This is incorrect as 2 casted to sc_uint, so it is unsigned in SV
        //z = sc_int<4>(-6) / sc_uint<4>(2);
        //std::cout << "z = " << z << std::endl;
        //CHECK(z == -3);
        // This is incorrect signed/unsigned mix, result is correct eventually
        //z = sc_int<4>(-6) / ux;
        //std::cout << "z = " << z << std::endl;
        //CHECK(z == -3);
        // The same problem, result is incorrect
        //z = ux / (-2);
        //std::cout << "z = " << z << std::endl;
        //CHECK(z == 0);
    } 
    
    void liter_neg_div_part() 
    {
        sc_int<4> x = 2;
        sc_int<6> y = -5;
        sc_int<8> z; sc_int<8> r;
        
        z = (-5) / 2;
        r = (-5) % 2;
        std::cout << "Liter:" << std::endl;
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == -2);
        CHECK(r == -1);
        
        z = sc_int<7>(-5) / sc_int<4>(2);
        r = sc_int<7>(-5) % sc_int<4>(2);
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == -2);
        CHECK(r == -1);
    }     
    
    void liter_neg_non_div() 
    {
        int i;
        sc_int<16> z;
        sc_bigint<16> bz;
        sc_uint<4> ux = 2;
        unsigned uu = 2;
        
        i = (-1) * (2);
        CHECK(i == -2);
        i = (-111) + 11;
        CHECK(i == -100);
        z = (-6) * 2;
        CHECK(z == -12);
        z = 4 * (-121);
        CHECK(z == -484);
        bz = (-10) - 20;
        CHECK(bz == -30);
        bz = -4 + (-8) - 2;
        CHECK(bz == -14);

        i = (-6) * ux;
        CHECK(i == -12);
        z = sc_int<4>(-6) * ux;
        CHECK(z == -12);
        
        bz = -3 + ux;
        CHECK(bz == -1); 
        bz = (-6) + ux * (-3);
        CHECK(bz == -12);

        i = (-6) * uu;
        CHECK(i == -12);
        z = sc_int<4>(-6) * uu;
        CHECK(z == -12);
        bz = (-6) + uu * (-3);
        CHECK(bz == -12);
    } 
    
    
// ---------------------------------------------------------------------------    
    
    // Division of negative number
    void cpp_type_neg_div() 
    {
        int x = 2;
        int y = -5;
        int z; int r; 
        z = y / x;
        r = y % x;
        std::cout << "CPP types:" << std::endl;
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == -2);
        CHECK(r == -1);
        
        // Incorrect results
//        unsigned ux = 2;
//        z = y / ux;
//        r = y % ux;
//        std::cout << "z = " << z << " r " << r << std::endl;
//        CHECK(z == 2147483645);
//        CHECK(r == 1);
    }

    // Division of negative number in @sc_int and @int
    void sc_type_neg_div() 
    {
        sc_int<5> x = 2;
        sc_int<7> y = -6;
        sc_int<8> z; 
        
        z = y / x;
        std::cout << "SC types:" << std::endl;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
        
        // Unsupported as gives different results in SC and SV
        sc_uint<4> ux = 2;
        z = y / ux;                 // Warning repotred
        std::cout << "z = " << z << std::endl;
        //CHECK(z == -3);
        
        int i = -6;
        z = i / ux;                 // Warning repotred
        std::cout << "z = " << z << std::endl;
        //CHECK(z == -3);

        unsigned uu = 2;
        z = y / uu;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
        
        z = i / uu;                 // Warning repotred
        std::cout << "z = " << z << std::endl;
        //CHECK(z == -3);
    }
    
    // Division of negative number in @sc_bigint
    void sc_bigint_type_neg_div() 
    {
        sc_bigint<9> y = -7;
        sc_int<5> x = 2;
        sc_int<11> z; 
        
        z = y / x;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
        
        sc_uint<4> ux = 2;
        z = y / ux;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
        
        sc_biguint<5> bux = 2;
        z = y / bux;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
        
        unsigned uu = 2;
        z = y / uu;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
        
        z = y / (sc_int<4>)2;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
    }
    
    // Division of @sc_biguint by negative number 
    void sc_biguint_type_neg_div() 
    {
        sc_biguint<9> y = 14;
        sc_int<5> x = -3;
        sc_int<11> z; 
        
        z = y / x;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -4);
        
        long int li = -3;
        z = y / li;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -4);
        
        sc_bigint<5> bx = -3;
        z = y / bx;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -4);
        
        sc_uint<4> ux = 5;
        z = y / (x + ux);       // Warning repotred
        //CHECK(z == 7);
        z = y / (bx + ux);
        std::cout << "z = " << z << std::endl;
        CHECK(z == 7);
        z = y * (bx - ux) + 1;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -111);
        z = (bx / ux) * y + y;
        std::cout << "z = " << z << std::endl;
        CHECK(z == 14);
    }
    
    // Positive number division and reminder
    void sc_type_pos_div_rem() 
    {
        sc_int<3> x = 2;
        sc_int<5> y = 5;
        sc_int<8> z; sc_int<8> r;
        
        z = y / x;
        r = y % x;
        std::cout << "SC types:" << std::endl;
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == 2);
        CHECK(r == 1);
        
        // Unsupported as gives different results in SC and SV
        sc_uint<4> ux = 2;
        z = y / ux;         // Warning reported
        r = y % ux;         // Warning reported
        //CHECK(z == 2);
        //CHECK(r == 1);
        
        sc_uint<4> uy = 5;
        z = uy / ux;
        r = uy % ux;
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == 2);
        CHECK(r == 1);
        
        sc_bigint<4> bx = 2;
        sc_bigint<4> by = 5;
        z = by / bx;
        r = by % bx;
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == 2);
        CHECK(r == 1);
        
        sc_biguint<4> bux = 2;
        sc_biguint<4> buy = 5;
        z = buy / bux;
        r = buy % bux;
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == 2);
        CHECK(r == 1);
    }

    // Division of negative number in @sc_int, same width
    void sc_type_neg_div2() 
    {
        sc_int<6> x = 2;
        sc_int<6> y = -5;
        sc_int<6> z;
        
        z = y / x;
        sc_int<6> r = y % x;
        std::cout << "SC types:" << std::endl;
        std::cout << "z = " << z << " r " << r << std::endl;
        CHECK(z == -2);
        CHECK(r == -1);
        
        y = -12;
        sc_int<6> ux = 3;
        z = y / ux;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -4);
    }
    
    // Check explicit cast to signed leads to signed in SV
    void expl_sign_cast()
    {
        int i = 42;
        sc_int<16> z;
        sc_bigint<16> bz;
        sc_int<6> x = -5;
        sc_bigint<6> bx = -5;
        sc_uint<4> ux = 2;
        sc_biguint<8> bux = 2;
        unsigned uu = 2;
    
        z = x + sc_int<7>(x);
        z = x + sc_int<7>(x+ux);        // Warning reported    
        z = x + ux;                     // Warning reported
        z = x + sc_uint<6>(ux);         // Warning reported
        z = x + sc_int<7>(ux);
        z = x + sc_bigint<8>(ux);
        z = x + sc_biguint<9>(ux);
        z = x + sc_uint<7>(ux);         // Warning reported
        z = x + int(ux);    
        z = x + (unsigned int)(ux);     // signed'({1'b0, 32'(ux)});
        
        bz = bx + sc_int<7>(x);
        bz = bx + sc_int<7>(x+ux);      // Warning reported    
        bz = bx + sc_int<7>(ux);
        bz = bx + sc_bigint<8>(ux);
        bz = bx + sc_biguint<9>(ux);
        bz = bx + sc_uint<7>(ux);
        bz = bx + int(ux);
        bz = bx + (unsigned int)(ux);

        bz = bux + sc_int<7>(x);
        bz = bux + i;
        bz = bux + sc_int<7>(ux);
        bz = bux + sc_bigint<8>(ux);
        bz = bux + int(ux);
        bz = bux + (unsigned int)(ux);
        
        bux += i;                       // Warning reported
        bux += sc_int<7>(x);            // Warning reported
        bux += sc_int<8>(ux);           // Warning reported
        bux += bux;             
        bux += sc_bigint<8>(bux);       // Warning reported
        bux += sc_biguint<9>(bux);
        
        bz  = -5;
        bz += uu;
        CHECK(z == -3);
        bz  = -5;
        bz += sc_int<7>(uu);
        CHECK(z == -3);
        bz  = -5;
        bz += ux;
        CHECK(z == -3);
        bz  = -5;
        bz += sc_int<7>(ux);
        CHECK(z == -3);
        bz  = -5;
        bz += sc_int<7>(bux);
        CHECK(z == -3);
        
        bx = -bux;
        bx = -bux + i;
        bx = -(sc_int<4>)bux + i;
        bx = -(sc_bigint<8>)bux + x;
        bx = -(sc_biguint<9>)bux + x;

        std::cout << "z = " << z << std::endl;
        CHECK(z == -3);
        z = x - int(ux) + 1;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -6);
        z = x * int(ux) + ux;           // Warning reported
        std::cout << "z = " << z << std::endl;
        //CHECK(z == -8);
        z = sc_bigint<13>(x) / sc_int<15>(ux);
        std::cout << "z = " << z << std::endl;
        CHECK(z == -2);
        z = long(ux) / (-1);
        std::cout << "z = " << z << std::endl;
        CHECK(z == -2);
    }
    

    // Comparison of negative number
    void sc_type_neg_comp() 
    {
        int i = 2;
        unsigned uu = 2;
        sc_int<4> x = 2;
        sc_int<5> y = 2;
        sc_uint<4> ux = 2;
        bool b;
        
        b = x == y;
        CHECK(b);
        b = ux == y;            // Warning reported
        //CHECK(b);
        b = i == y;
        CHECK(b);
        b = i == x;
        CHECK(b);               
        b = i == uu;            // Warning reported
        //CHECK(b);
        b = x == uu;
        CHECK(b);
        b = ux == uu;
        CHECK(b);
        
        x = -5; y = -5;
        i = -5;
        b = x == y;
        CHECK(b);
        b = i == y;
        CHECK(b);
        
        b = i == ux;        // Warning reported
        CHECK(!b);  
        b = y == ux;        // Warning reported
        CHECK(!b);
    }
    
    // 64-bit SC types
    // If result variable of signed/unsigned mix is less 64bit, it got correct
    // sign from UUL which is 64bit
    void sc_types_64bit() 
    {
        sc_int<64> x = -5;
        sc_uint<64> ux = 3;
        int i = -5;
        unsigned uu = 3;
        sc_biguint<60> y = 3;
        sc_bigint<80>z;
        
        z = x * ux;         // Warning reported
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 18446744073709551601ULL); -15 in SV
        z = x * sc_int<64>(ux);
        std::cout << "z = " << z << std::endl;
        CHECK(z == -15);
        
        z = x / ux;             // Warning reported
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 6148914691236517203ULL); -1 in SV
        z = x / sc_int<64>(ux);
        std::cout << "z = " << z << std::endl;
        CHECK(z == -1);
        
        z = x * uu;
        CHECK(z == -15);
        z = x / uu;
        CHECK(z == -1);
        z = i * ux;     // Warning reported
        std::cout << "z = " << z << std::endl;
        // CHECK(z == 18446744073709551601ULL); -15 in SV
        z = i / ux;     // Warning reported
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 6148914691236517203ULL); -1 in SV
    }
    
// --------------------------------------------------------------------------    
    
    template <typename T1, typename T2, typename T3 = int>
    void cpp_sc_type_mix(int div, int rem, int mul, int sub, int add, 
                         bool doDiv = true) 
    {
        T1 x = 3;
        T2 i = -10;
        
        T3 z;
        if (doDiv) {
            z = i / x;
            std::cout << "z " << z << std::endl;  
            CHECK(z == div);
            z = i % x;
            std::cout << "z " << z << std::endl;  
            CHECK(z == rem);
        }
        z = i * x;          // Warning reported
        std::cout << "z " << z << std::endl;  
        //CHECK(z == mul);
        z = i - x;          // Warning reported
        std::cout << "z " << z << std::endl;  
        //CHECK(z == sub);
        z = i + x;          // Warning reported
        std::cout << "z " << z << std::endl;  
        //CHECK(z == add);
    }
    
   
    void cpp_sc_type_mix() 
    {
        cpp_sc_type_mix<int, int>(-3, -1, -30, -13, -7);
        
        cpp_sc_type_mix<unsigned, int>(1431655762, 0, -30, -13, -7, false);
        cpp_sc_type_mix<unsigned, sc_int<8>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<unsigned, sc_bigint<8>, sc_bigint<16>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<unsigned, sc_int<33>, sc_int<64>>(-3, -1, -30, -13, -7);
       
        // No div, rem
        cpp_sc_type_mix<sc_uint<8>, int>(1431655762, 0, -30, -13, -7, false);
        // No div, rem
        cpp_sc_type_mix<sc_uint<8>, sc_int<8>>(1431655762, 0, -30, -13, -7, false);
        // No div, rem
        cpp_sc_type_mix<sc_uint<8>, sc_int<8>, sc_int<8>>(82, 0, -30, -13, -7, false);
        cpp_sc_type_mix<sc_uint<8>, sc_bigint<8>, sc_bigint<16>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<sc_uint<8>, sc_int<33>, sc_int<64>>(-3, -1, -30, -13, -7, false);
        cpp_sc_type_mix<sc_uint<33>, sc_int<33>, sc_int<64>>(-3, -1, -30, -13, -7, false);
        cpp_sc_type_mix<sc_uint<8>, sc_bigint<33>, sc_bigint<64>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<sc_uint<33>, sc_bigint<33>, sc_bigint<64>>(-3, -1, -30, -13, -7);

        cpp_sc_type_mix<sc_biguint<8>, int, sc_bigint<16>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<sc_biguint<8>, sc_int<8>, sc_bigint<16>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<sc_biguint<8>, sc_bigint<8>, sc_bigint<16>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<sc_biguint<8>, sc_bigint<33>, sc_bigint<65>>(-3, -1, -30, -13, -7);
        cpp_sc_type_mix<sc_biguint<33>, sc_bigint<33>, sc_bigint<65>>(-3, -1, -30, -13, -7);
    }
    
    template <typename T1, typename T2, typename T3, typename T4 = int>
    void compl_expr(int div, int mul, int add, int sub, int add2,
                    bool doDiv = true) 
    {
        T1 x = -24;
        T2 y = 6;
        T3 z = 2;
        T4 r;
        
        if (doDiv) {
            r = (x/y) / z;          // Warning reported
            std::cout << "r " << r << std::endl;  
            //CHECK(r == div);

            r = (x * y) / z;        // Warning reported
            std::cout << "r " << r << std::endl;  
            //CHECK(r == mul);

            r = (x + y) / z;        // Warning reported
            std::cout << "r " << r << std::endl;  
            //CHECK(r == add);

            r = (x / y) - z;        // Warning reported
            std::cout << "r " << r << std::endl;  
            //CHECK(r == sub);
        }
        
        r = (x * y) + z;
        std::cout << "r " << r << std::endl;  
        //CHECK(r == add2);
    }
    
    void compl_expr_mix() 
    {
        compl_expr<int, int, unsigned>(2147483646, 2147483576, 2147483639, -6, -142, false);
        compl_expr<sc_int<8>, sc_int<8>, unsigned>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<8>, sc_int<33>, unsigned>(-2, -72, -9, -6, -142);
        // r2 all
        compl_expr<sc_int<8>, sc_int<8>, long unsigned>(-2, -72, -9, -6, -142);
        
        compl_expr<sc_int<8>, sc_bigint<8>, unsigned, sc_bigint<16>>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<33>, sc_bigint<33>, unsigned, sc_bigint<65>>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<8>, int, unsigned>(-2, -72, -9, -6, -142);
        compl_expr<sc_bigint<8>, sc_bigint<8>, unsigned, sc_bigint<16>>(-2, -72, -9, -6, -142);
        
        // r6 all
        compl_expr<int, int, sc_uint<8>>(-2, -72, -9, -6, -142);
        compl_expr<int, int, sc_uint<33>>(-2, -72, -9, -6, -142);
        // r7 all
        compl_expr<sc_int<8>, sc_int<8>, sc_uint<8>>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<8>, sc_bigint<8>, sc_uint<8>, sc_bigint<16>>(-2, -72, -9, -6, -142);
        // r9 all
        compl_expr<sc_int<8>, int, sc_uint<8>>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<33>, int, sc_uint<8>>(-2, -72, -9, -6, -142);
        compl_expr<sc_bigint<8>, sc_bigint<8>, sc_uint<8>, sc_bigint<16>>(-2, -72, -9, -6, -142);
        
        compl_expr<int, int, sc_biguint<8>, sc_bigint<16>>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<8>, sc_int<8>, sc_biguint<8>, sc_bigint<16>>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<8>, sc_bigint<8>, sc_biguint<8>, sc_bigint<16>>(-2, -72, -9, -6, -142);
        compl_expr<sc_int<8>, int, sc_biguint<8>, sc_bigint<16>>(-2, -72, -9, -6, -142);
        compl_expr<sc_bigint<8>, sc_bigint<8>, sc_biguint<8>, sc_bigint<16>>(-2, -72, -9, -6, -142);
        compl_expr<sc_bigint<33>, sc_bigint<65>, sc_biguint<65>, sc_bigint<65>>(-2, -72, -9, -6, -142);
        
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

