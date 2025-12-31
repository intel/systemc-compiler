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

using namespace sct;
        
// Operations with negative numbers of C++ and SC types, 
// mixed of signed and unsigned types -- should not be normally used
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<sc_uint<32> > s{"s"};
    sc_signal<sc_uint<1>>   z{"z"};
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(github_16); sensitive << sa << sb;
        SC_METHOD(liter_suff_U2); sensitive << s;
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

        SC_METHOD(liter_suff_U); sensitive << s;
        SC_METHOD(self_determined_expr); sensitive << s << z;
        SC_METHOD(cpp_overflow); sensitive << s;
        SC_METHOD(cpp_overflow2); sensitive << s;
        SC_METHOD(cpp_overflow3); sensitive << s;
        SC_METHOD(sc_unsigned_error); sensitive << s << sbu1 << sbu2;
        SC_METHOD(others); sensitive << s;
        
        SC_METHOD(assign_s);
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    sc_signal<uint16_t> sa;
    sc_signal<uint16_t> sb;

    sc_signal<int> t0;
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
        t0 = us_result;
    }
    
    // Literal suffix @U just ignored
    sc_signal<int> t1;
    void liter_suff_U2() 
    {    
        sc_int<3> x = 2;
        t1 = x;
        sc_int<4> y = -6;
        t1 = y;
        sc_int<8> z;
        z = 6 / 2U;
        t1 = z;
        CHECK(z == 3);
        z = (-6) / 2U;      // Warning repotred
        t1 = z;
        //CHECK(z == -3);

        z = 12U / 6;
        t1 = z;
        CHECK(z == 2);
        z = 12U / (-6);     // Warning repotred
        t1 = z;
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 0);  -2 in SV
    }
    
    // Literal suffix @L just ignored
    sc_signal<int> t2;
    void liter_suff_L() 
    {
        sc_int<3> x = 2;
        t2 = x;
        sc_int<4> y = -6;
        t2 = y;
        sc_int<8> z;
        sc_bigint<70> bz;
        z = 6 / 2L;
        t2 = z;
        CHECK(z == 3);
        z = (-6) / 2L;
        t2 = z;
        CHECK(z == -3);

        z = 12L / 6;
        t2 = z;
        CHECK(z == 2);
        z = 12L / (-6);
        t2 = z;
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
    
    sc_signal<int> t3;
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
        t3 = z;
        std::cout << "z = " << z << std::endl;
        //CHECK(z == -2); 
        
        i = -1;
        i = j + uu * i;         // Warning repotred
        CHECK(i == -38); 
        
        z = 1;
        bz = (bu * bz - x);
        
        z = -6 / ux;            // Warning repotred
        t3 = z;
        bz = (-6) + ux * (-3);
        t3 = bz.to_int();
        //CHECK(bz == -12);  
        
        // Unsigned division, incorrect expression
        z = (-uu*4) / (-2);     // Warning repotred
        t3 = z;
        std::cout << "z = " << z << std::endl;
        CHECK(z == 0); 
        
        // Literal expression
        uu = ux + (1 + 2);      // Warning repotred
        t3 = uu;
        uu = ux / (1 << 2);     // Warning repotred
        t3 = uu;
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
    
    // Literal suffix @U just ignored
    sc_signal<sc_bigint<64>> tt;
    const sc_int<32> S1      = "2";   
    const sc_uint<32> S2     = "0x2"; 
    const unsigned U1        = 2U;    
    static const unsigned U2 = 2UL;   
    
    template<class T>
    void liter_suff_U_T() 
    {
        T x;
        x = 0;                      // Simple form
        x = 1;                      // Simple form
        
        x = -3;                     // OK: 'sd 
        x = 3;                      // OK: simple
        x = x + 3;                  // OK: simple or `d
        x = 3 > x;                  // OK: simple or `d

        x = -4U;                    // OK: 'sd 
        x = 4U;                     // OK: 'd or 'sd 
        x = x + 4U;                 // OK: 'd or simple
        x = 4U > x;                 // OK: 'd or simple
        
        x = -5;                     // OK: 'sd
        x = sc_uint<8>(5);          // OK: 'd or 'sd      
        x = sc_uint<8>(5)+x;        // OK: 'd or 'sd      
        x = sc_uint<8>(5) > x;      // OK: 'd       
        
        x = -sc_int<8>(6);          // OK: 'sd
        x = sc_int<8>(6);           // !!! OK: 'd or 'sd 
        x = sc_int<8>(6)+x;         // OK: 'sd
        x = sc_int<8>(6) > x;       // !!! OK: 'sd

        x = sc_uint<5>("42");
        x = sc_uint<2>(U1-1U-0);
        x = sc_uint<2>(U1-1U-0) > x;
        
        x = U1 + U2 + S1 + S2;
        tt = sc_bigint<64>(x);
    }
    
    void liter_suff_U() {
        bool x;
        sc_uint<1> y;
        
        liter_suff_U_T<int>();
        liter_suff_U_T<unsigned>();
        liter_suff_U_T<sc_int<64>>();
        liter_suff_U_T<sc_uint<64>>();
        liter_suff_U_T<sc_bigint<64>>();
        liter_suff_U_T<sc_biguint<64>>();
        
        tt = x + y;
    }
    
    
    // Check no overflow for self determined expression with literals
    void self_determined_expr() {
        using namespace sct;
      
        sct_uint<1> k = 0;
        sct_uint<2> i = 3;
        sct_uint<3> j = 5;
        sct_uint<65> bi = "0x10000000000000000";
        sct_uint<65> bj = "0x10000000000000000";
        unsigned res; bool bres;
        
        res = 1U << (i + j);             // OK    
        cout << "1 << (i + j) " << res << endl;
        sct_assert(res == 256);
        
        res = 0x1000U >> (i + j);        // OK    
        cout << "1 >> (i + j) " << res << endl;
        sct_assert(res == 16);

        k = 1;
        res = (i + j) + k;              // OK    
        cout << "(i + j) + k " << res << endl;
        sct_assert(res == 9);
        k = 0;

        auto x1 = 1 << (2U+2U); 
        cout << "x1 " << x1 << endl;
        CHECK(x1 == 16);
        
        auto x2 = 1 << (2U*2U);
        cout << "x2 " << x2 << endl;
        CHECK(x2 == 16);
        
        
        // Comparison operators
        bres = (i + j == k);            // FIXED
        cout << "i + j == k " << bres << endl;
        sct_assert(!bres);

        bres = (2U + 2U == k);            // FIXED
        cout << "2U + 2U == k " << bres << endl;
        sct_assert(!bres);        
        
        bres = (i + j > z.read());      // FIXED
        cout << "i + j > z " << bres << endl;
        sct_assert(bres);
        
        bres = (bi + bj == k);            // FIXED
        cout << "bi + bj == k " << bres << endl;
        sct_assert(!bres);
        
        bres = (bi + bj > z.read());      // FIXED
        cout << "bi + bj > z " << bres << endl;
        sct_assert(bres);
        
        
        // Condition in ternary operator
        res = (i + j + z.read()) ? 1 : 2;          // FIXED
        cout << "(i + j) ? 1 : 2 " << res << endl;
        sct_assert(res == 1);
        
        res = (2U + 2U + z.read()) ? 1 : 2;          
        cout << "(2U + 2U) ? 1 : 2 " << res << endl;
        sct_assert(res == 1);
        
        // Condition in terminators
        if (i + j + z.read()) res = 1; else res = 2;  // FIXED
        cout << "if (i + j) ... " << res << endl;
        sct_assert(res == 1);
        
        // Expression assigned to bool LHS
        bres = i + j + z.read();         // FIXED
        cout << "bool b = i + j... " << bres << endl;
        sct_assert(bres);

        bres = 2U + 2U + z.read();         // FIXED
        cout << "bool b = 2U + 2U... " << bres << endl;
        sct_assert(bres);

        // Expression casted to bool
        res = bool(i + j) + 1;           // FIXED   
        cout << "res = bool(i + j) + 1 " << res << endl;
        sct_assert(res == 2);
  

        // Logical OR/AND/...
        bres = (i + 2U + z.read()) && (i + j);     // FIXED
        cout << "(i + 2U) || (i + j) " << bres << endl;
        sct_assert(bres);

        bres = !(!(i + j + z.read()) || !(2U + j));     // FIXED
        cout << "!(i + j) && (2U + j) " << bres << endl;
        sct_assert(bres);

        // Biwise OR/AND/...
        res = (i + j + z.read()) | (i + j);      // FIXED
        cout << "(i + j) | (i + j) " << res << endl;
        sct_assert(res);
        
        auto wres = (bi + bj + z.read()) | (bi + bj);      // FIXED
        cout << "(bi + bj) | (bi + bj) " << hex << wres << dec << endl;
        sct_assert(wres == sc_biguint<66>("0x20000000000000000"));
        
        // Expression assigned to narrow width LHS
        sc_uint<1> s = i + j + z.read();  // OK
        cout << "bool b = i + j... " << s << endl;
        sct_assert(s == 0);
        
        // Check ++ and -- for width increase
        i = 3;
        bres = (++i == k);            
        cout << "++i == k " <<  i << " " << bres << endl;
        sct_assert(bres);

        unsigned u = 0;
        bres = (--u == k);            
        cout << "--u == k " << u << " " << bres << endl;
        sct_assert(!bres);

        // Others
        i = 3;
        res = 1 << (i + 1);
        cout << "1 << (i + 1) " << res << endl;
        CHECK(res == 16);
        res = 1 << (i * 2);
        cout << "1 << (i * 2) " << res << endl;
        CHECK(res == 64);

        res = 1 << (2*i + j - 1);
        CHECK(res == 1024);
        
        // Check for width determination in complex expression
        j = 7;
        res = (j+1) + (i*2);
        CHECK(res == 14);
        
        bres = (j+1 == 0) || (i*2 == 0);
        CHECK(!bres);
    }
    
    sc_signal<sc_biguint<70>> sbu1;
    sc_signal<sc_biguint<70>> sbu2;
    sc_signal<unsigned> tt2;
    void sc_unsigned_error() {
        sct_uint<65> bi;
        sct_uint<65> bj;
        // Cannot determine width of @sc_unsigned -- error reported
        //bi = s.read() | (sbu1.read() << sbu2.read());  // ERROR

        // That should be rewritten in the following form
        bi = sbu1.read() << sbu2.read();
        bj = s.read() | bi;
        
        //auto ci = sbu1.read() << sbu2.read();       // ERROR
        //bj = s.read() | ci;                     
        
        tt2 = bj.to_uint();
    }    
    
    // C++ overflow example
    sc_signal<unsigned> tt1;
    void cpp_overflow() {
        using namespace sct;
        
        unsigned i1 = 1U << 31;
        unsigned i2 = 2;
        int si1 = 1U << 31;
        int si2 = -2;
        sc_uint<64> y1 = 1ULL << 63;
        sc_uint<64> y2 = 2;
        sc_int<64> sy1 = 1ULL << 63;
        sc_int<64> sy2 = -2;
        sc_biguint<64> x1 = 1ULL << 63;
        sc_biguint<64> x2 = 2;
        sc_bigint<64> sx1 = 1ULL << 63;
        sc_bigint<64> sx2 = -2;
        sc_biguint<68> x3;
        sc_bigint<68> sx3;
        
        // CPP types, #330
        x3 = i1 * i2;
        cout << hex << "(i1 * i2) " << x3  << dec << endl;
        CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        x3 = si1 * si2;
        cout << hex << "(si1 * si2) " << x3  << dec << endl;
        CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        x3 = i1 * si2;
        cout << hex << "(i1 * si2) " << x3  << dec << endl;
        CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        // SC types, #330
        x3 = y1 * y2;
        cout << hex << "(y1 * y2) " << x3  << dec << endl;
        CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        x3 = sy1 * sy2;
        cout << hex << "(sy1 * sy2) " << x3  << dec << endl;
        CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        x3 = y1 * sy2;
        cout << hex << "(y1 * sy2) " << x3  << dec << endl;
        CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        // big SC types
        x3 = x1 * x2;
        cout << hex << "(x1 * x2) " << x3  << dec << endl;
        CHECK(x3 == sc_biguint<68>("0x10000000000000000"));

        sx3 = sx1 * sx2;
        cout << "(sx1 * sx2) " << hex << sx3 << dec << " : " << sx3 << endl;
        CHECK(sx3 == sc_bigint<68>("0x10000000000000000"));

        sx3 = x1 * sx2;
        cout << hex << "(x1 * sx2) " << sx3  << dec << endl;
        CHECK(x3 == sc_biguint<68>("0x10000000000000000"));
        
        // Unary 
        i1 = 0xFFFFFFFF;
        si1 = -2147483648;
        x3 = ++i1;
        cout << hex << "i1++ " << x3 << dec << endl;
        CHECK(x3 == 0);  

        x3 = --si1;
        cout << hex << "si1-- " << x3 << dec << endl;
        CHECK(x3 == 0x7FFFFFFF);  
        
        y1 = 0xFFFFFFFFFFFFFFFF;
        sy1 = -0x8000000000000000;
        x3 = ++y1;
        cout << hex << "y1++ " << x3 << dec << endl;
        CHECK(x3 == 0);  

        x3 = --sy1;
        cout << hex << "sy1-- " << x3 << dec << endl;
        CHECK(x3 == 0x7FFFFFFFFFFFFFFF);
    }
    
    void cpp_overflow2() {
        using namespace sct;
        
        unsigned i1 = 1U << 31;
        unsigned i2 = 2;
        int si1 = 1U << 31;
        int si2 = -2;
        sc_uint<16> y1 = 3;
        sc_uint<16> y2 = 2;
        sc_int<16> sy = 3;
        sc_int<16> sy1 = -3;
        sc_int<16> sy2 = -2;
        sc_biguint<16> x1 = 3;
        sc_biguint<16> x2 = 2;
        sc_bigint<16> sx1 = -3;
        sc_bigint<16> sx2 = -2;
        sc_biguint<42> x3;
        sc_bigint<42> sx3;
        
        // SC types, #330
        x3 = y1 + sy2;  // Warning reported
        cout << hex << "(y1 + sy2) " << x3  << dec << endl;
        //CHECK(x3 == 1);  //0x1 in SC, 0x10...1 in SV
        
        x3 = y2 + sy2;
        cout << hex << "(y2 + sy2) " << x3  << dec << endl;
        //CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        x3 = sy + sy2;
        cout << hex << "(sy + sy2) " << x3  << dec << endl;
        CHECK(x3 == 1);  //
        
        sx3 = y2 + sy2;
        cout << hex << "(y2 + sy2) " << x3  << dec << endl;
        //CHECK(x3 == 0);  //0 in SC, 0x10...0 in SV

        x3 = x1 + sx2;
        cout << hex << "(x1 + sx2) " << x3  << dec << endl;
        CHECK(x3 == 1);  // OK

        sx3 = x2 + sx1;
        cout << hex << "(x2 + sx1) " << sx3  << dec << endl;
        CHECK(sx3 == -1);  // OK

        x3 = x2 + sx2;
        cout << hex << "(x2 + sx2) " << x3  << dec << endl;
        CHECK(x3 == 0);  // OK

        sx3 = x2 + sx2;
        cout << hex << "(x2 + sx2) " << sx3  << dec << endl;
        CHECK(sx3 == 0);  // OK
    }
    
    void cpp_overflow3() {
        using namespace sct;
        
        cout << "cpp_overflow3 "  << endl;
        sc_uint<7> k, m;
        k = 41; m = 42;
        sc_uint<8> res;

        res = (k - m) % 11;   // Non-equivalent
        cout << "res " << res << endl;
        //CHECK(res == 4);  

        res = sc_uint<8>(k - m) % 11;    // OK
        cout << "res " << res << endl;
        CHECK(res == 2);  
        
        unsigned u = 1;
        res = u << 32;    // Non-equivalent: 1 in SC, 0 in SV
    }

    sc_signal<unsigned> tt3;
    void others() {
        int a = -5;
        unsigned u = 1;
        bool c;
        sct_uint<1> k = 0;
        sct_uint<2> i = 3;
        sct_uint<3> j = 5;
        sct_uint<65> bi = "0x10000000000000000";
        sct_uint<65> bj = "0x10000000000000000";
        sc_biguint<66> res;

        // Overflow checks -- work well for SC types
        // --- 1 ---
        c = (i + j == k);
        cout << "i + j == k " << c << endl;
        sct_assert(!c);     // OK
        
        // --- 2 ---
        c = (bi + bj == k);            
        cout << "bi + bj == k " << c << endl;
        sct_assert(!c);     // OK
        
        
        // Cast for left shift -- C++ data types overflow not supported!!!
        unsigned m0;
        m0 = 31;
        c = (1U << m0);
        //cout << "1 << m0 " << hex << (1U << m0) << dec << " " << c << endl;
        //sct_assert(c);

        m0 = 64;
        c = (1ULL << m0);
        //cout << "1ULL << m0 " << hex << (1U << m0) << dec << " " << c << endl;
        
        m0 = 32;            
        res = (1U << m0);     // Cyclic shift in SC results is 1, 0 in SV
        //cout << "1 << m0 " << hex << (1U << m0) << dec << " " << c << endl;
        //sct_assert(c);      // ERROR

        m0 = 32;
        res = (sc_uint<32>(1) << m0);
        c = (sc_uint<32>(1) << m0);
        cout << "1 << m0 " << hex << res << dec << " " << c << endl;
        sct_assert(c);       // OK

        m0 = 64;
        res = (sc_biguint<66>(1) << m0);
        c = res != 0;           
        cout << "<66>1 << m0 " << hex << res << dec << " " << c << endl;
        sct_assert(c);       // OK
    }

    void assign_s() {
        s = 0;
        z = 0;
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

