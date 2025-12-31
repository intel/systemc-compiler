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

// Binary/unary/compound complex expressions with negative numbers of 
// C++ and SC types to check correctness of "signed" applied
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<sc_uint<32> > s{"s"};
    
    SC_CTOR(A)
    {
        SC_METHOD(compare_operators); sensitive << s;
        SC_METHOD(compare_operators_literals); sensitive << s;
        SC_METHOD(compare_operators_in_cond); sensitive << s;
        SC_METHOD(binary); sensitive << s;
        SC_METHOD(binary_liter); sensitive << s;
        SC_METHOD(compound); sensitive << s;
        SC_METHOD(unary); sensitive << s;
        SC_METHOD(ternary); sensitive << s;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    enum EI {ei1 = -3, ei2 = -4, ei3  =4};
    enum EU : unsigned {eu1, eu2, eu3};

    // Signed/unsigned comparison operators: >, <, ==, != and others
    void compare_operators() 
    {
        bool z;
        sc_bigint<20> bz;

        sc_int<4>  x = -4;
        sc_uint<4> ux = 2;
        sc_bigint<10> bx = -7;
        sc_int<5>  y = 2;
        sc_uint<5>  uy = 2;
        sc_bigint<11> by = 8;
        sc_biguint<11> buy = 8;
        int i = -5;
        unsigned uu = 6;
        unsigned long ul = 6;

        // Mix results in unsigned comparison
        z = x < ux; cout << "x < ux " << z <<endl; CHECK(!z);
        z = y == uy; cout << "y == uy " << z <<endl; CHECK(z);
        z = y > uy; cout << "y > uy " << z <<endl; CHECK(!z);
        z = i < uu; cout << "i < uu " << z <<endl; CHECK(!z);

        // Mix for big SC types results in signed comparison
        z = bx < by; cout << "bx < by " << z <<endl; CHECK(z);
        z = bx < uy; cout << "bx < uy " << z <<endl; CHECK(z);  
        bz = bx + uy; cout << "bx + uy " << bz <<endl; CHECK(bz == -5);
        z = bx < uu; cout << "bx < uu " << z <<endl; CHECK(z);  
        z = buy > i; cout << "buy > i " << z <<endl; CHECK(z);  
        z = buy > x; cout << "buy > x " << z <<endl; CHECK(z);  
        z = buy > i; cout << "buy > i " << z <<endl; CHECK(z);  
        z = uu >= bx; cout << "uu >= bx " << z <<endl; CHECK(z);

        // Mix for SC int type and unsigned 32bit results in signed comparison
        z = uu > x; cout << "uu >= x " << z <<endl; CHECK(z);
        z = ul > x; cout << "ul >= x " << z <<endl; CHECK(!z);

        // Big SC unsigned and enum
        z = buy > ei2; cout << "buy > ei2 " << z <<endl; CHECK(z);
        z = buy > eu2; cout << "buy > eu2 " << z <<endl; CHECK(z);

         // Unsigned comparison
        z = ux < buy; cout << "ux < buy " << z <<endl; CHECK(z);
        z = ux+uu == buy; cout << "ux+uu == buy " << z <<endl; CHECK(z);

        // Signed comparison
        z = i < x; cout << "i < x " << z <<endl; CHECK(z);
        z = x > bx; cout << "x > bx " << z <<endl; CHECK(z);
    }

     void compare_operators_literals() 
    {
        bool z;
        sc_bigint<20> bz;

        sc_int<4>  x = -4;
        sc_uint<4> ux = 2;
        sc_bigint<10> bx = -7;
        sc_int<5>  y = 2;
        sc_uint<5>  uy = 2;
        sc_bigint<11> by = 8;
        sc_biguint<11> buy = 8;
        int i = -5;
        unsigned uu = 6;

        // Mix results in unsigned comparison
        z = x < 2ULL; cout << "x < ux " << z <<endl; CHECK(!z);
        z = 2 == uy; cout << "y == uy " << z <<endl; CHECK(z);
        z = y > uy; cout << "y > uy " << z <<endl; CHECK(!z);
        z = i < 6U; cout << "i < uu " << z <<endl; CHECK(!z);

        // Mix for big SC types results in signed comparison
        z = bx < 8; cout << "bx < by " << z <<endl; CHECK(z);
        z = bx < 2U; cout << "bx < uy " << z <<endl; CHECK(z);  
        z = bx < 6U; cout << "bx < uu " << z <<endl; CHECK(z);  
        z = buy > -5; cout << "buy > i " << z <<endl; CHECK(z);  
        z = buy > -4; cout << "buy > x " << z <<endl; CHECK(z);  
        z = sc_biguint<11>(8) > i; cout << "buy > i " << z <<endl; CHECK(z);  
        z = 6U >= bx; cout << "uu >= bx " << z <<endl; CHECK(z);

        // Mix for SC int type and unsigned 32bit results in signed comparison
        z = 6U > x; cout << "uu >= x " << z <<endl; CHECK(z);

        // Unsigned comparison
        z = 2U < buy; cout << "ux < buy " << z <<endl; CHECK(z);
        z = ux+6U == buy; cout << "ux+uu == buy " << z <<endl; CHECK(z);

        // Signed comparison
        z = -5 < x; cout << "i < x " << z <<endl; CHECK(z);
        z = -4 > bx; cout << "x > bx " << z <<endl; CHECK(z);

        // There is signed cast if literal is signed
        z = buy.range(4,1) == 4; cout << "x < ux " << z <<endl; sct_assert(z);
        z = buy.range(4,1) == 4U; cout << "x < ux " << z <<endl; sct_assert(z);
        z = buy.range(4,1) > -5; cout << "x < ux " << z <<endl; sct_assert(z);
    }

    sc_signal<unsigned> t2;
    void compare_operators_in_cond() {
        sc_int<4>  x = -4;
        sc_uint<4> ux = 2;
        sc_bigint<10> bx = -7;
        sc_int<5>  y = 2;
        sc_uint<5>  uy = 2;
        sc_bigint<11> by = 8;
        sc_biguint<11> buy = 8;
        int i = -5;
        unsigned uu = 6;
       
        if (ux == s.read()) {
            t2 = 1;
        }
        for (unsigned i = 0; i < 3; ++i) {
            t2 = i;
        }      

        if (x < ux) {
            cout << "x < ux " << (x < ux) << endl; CHECK(0);
        }
        if (y == uy) {} else {
            cout << "y == uy " << (y == uy) << endl; CHECK(0);
        }
        if (y > uy) {
            cout << "y > uy " << (y > uy) << endl; CHECK(0);
        }
        if (i < uu) {
            cout << "i < uu " << (i < uu) << endl; CHECK(0);
        }

        // Mix for big SC types results in signed comparison
        if (bx < by) {} else {
            cout << "bx < by " << (bx < by) << endl; CHECK(0);
        }
        if (bx < uy) {} else {
            cout << "bx < uy " << (bx < uy) << endl; CHECK(0);
        }        
        if (bx < uu) {} else {
            cout << "bx < uu " << (bx < uu) << endl; CHECK(0);
        }
        if (buy > i) {} else {
            cout << "buy > i " << (buy > i) << endl; CHECK(0);
        }
        if (buy > x) {} else {
            cout << "buy > x " << (buy > x) << endl; CHECK(0);
        }
        while (buy < i) {
            cout << "buy < i " << (buy > i) << endl; CHECK(0);
        }
        if (uu >= bx) {} else {
            cout << "uu >= bx " << (uu >= bx) << endl; CHECK(0);
        }

        if (ux < buy) {} else {
            cout << "ux < buy " << (ux < buy) << endl; CHECK(0);
        }
        if (ux + uu == buy) {} else {
            cout << "ux+uu == buy " << (ux+uu == buy) << endl; CHECK(0);
        }

        if (i < x) {} else {
            cout << "i < x " << (i < x) << endl; CHECK(0);
        }
        if (x > bx) {} else {
            cout << "x > bx " << (x > bx) << endl; CHECK(0);
        }

        ux = s.read();
        by = s.read();
        uu = s.read();

        bool z;
        z = ux == 0U;   // No cast
        z = ux < by;    // cast
        z = uu < y;     // cast
        z = buy > x;    // cast

    }

    void binary() 
    {
        sc_int<16> z;
        sc_bigint<16> bz;
        sc_biguint<16> uz = 7;
        sc_int<4>  x = -4;
        sc_uint<4> ux = 2;
        int i = -5;
        unsigned uu = 7;

        z = (ux + x) * (-1);                        // Warning reported
        //CHECK(z == 2);  
        z = (ux - x) * x;                           // Warning reported
        //CHECK(z == -24);
        z = (ux * (x + 1)) - ux;                    // Warning reported
        //CHECK(z == -8);  
        
        bz = (-6) + ux * (-3);
        CHECK(bz == -12);  
        bz = -x + ux * 2;
        std::cout << "z = " << bz << std::endl;
        //CHECK(bz == 8);  
        
        bz = x + ux * i;                            // Warning reported
        std::cout << "z = " << bz << std::endl;
        //CHECK(bz == -14); 
        bz = x * (ux - i);                          // Warning reported
        std::cout << "z = " << bz << std::endl;
        //CHECK(bz == -28); 
        bz = 14 - (ux - i + 3);                     // Warning reported
        std::cout << "z = " << bz << std::endl; 
        CHECK(bz == 4); 
        
        z = 1; bz = -5;
        bz = (uz * bz - x);
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == -31);
        bz = -5;
        bz = (z * bz - x);
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == -1);
        bz = -5;
        bz = (z * (bz + 1) - bz);
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == 1);

        z = (uu*4) - i;                             // Warning reported
        std::cout << "z = " << z << std::endl;
        CHECK(z == 33);  
        z = (uu*4) - i + x * ux;                    // Warning reported
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 25);  
        
        bz = x/(i-1) * (uu - ux);                   // Warning reported
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == 0);  
        bz = x/(i-1) * (uu - ux + 1);               // Warning reported
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == 0);  
    }
    
    
    void binary_liter() 
    {
        sc_int<16> z;
        sc_bigint<16> bz;

        sc_int<4>  x = -4;
        sc_uint<4> ux = 2;
        z = (ux - x) * x + 1;                       // Warning reported
        //CHECK(z == -23);
        z = (ux * (x + 1)) - 7;                     // Warning reported
        //CHECK(z == -13);  
        
        bz = (-6) + ux * (-3) * 2;
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == -18);  
        bz = -x + ux * 2 + 1;                       // Warning reported
        //CHECK(bz == 9);  
        
        int i = -5;
        unsigned uu = 7;

        z = ((uu*4) - i) / 1;                       // Warning reported
        std::cout << "z = " << z << std::endl;
        CHECK(z == 33);  
        z = (4*x + i) / (-2);
        std::cout << "z = " << z << std::endl;
        CHECK(z == 10);  
        z = (uu*4) - i + x * ux + 1;                // Warning reported
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 26);  
        
        bz = x/(i-1) * (uu - ux + 1);               // Warning reported
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == 0);  
    }
    
    void compound() 
    {
        int z;
        sc_bigint<33> bz;

        sc_int<7>  x = -4;
        sc_uint<7> ux = 2;
        z = 1;
        z += (ux + x) * (-1);                       // Warning reported
        std::cout << "z = " << z << std::endl;
        CHECK(z == 3);  
        ux *= (ux - x + 1);                         // Warning reported
        std::cout << "z = " << ux << std::endl;
        CHECK(ux == 14);
        x = (ux * (x + 1)) - ux;                    // Warning reported
        std::cout << "z = " << x << std::endl;
        CHECK(x == -56);  
        
        bz = -100;
        bz /= (-6) + x * (-3);
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == 0);  
        bz = -2;
        bz *= -x + ux * 2;                          // Warning reported
        std::cout << "z = " << bz << std::endl;
        //CHECK(bz == -168);  
        
        int i = -5; x = -4;
        unsigned uu = 7; ux = 2;
        bz = 1;
        bz -= x/(i+1) * (uu - ux + 1);              // Warning reported
        std::cout << "bz = " << bz << std::endl;
        //CHECK(bz == -5);  
    }
    
    sc_signal<int> t0;
    void unary() 
    {
        int z;
        sc_bigint<33> bz;
        int i = -5;
        unsigned uu = 7;
        t0 = uu;

        sc_int<7>  x = -4;
        sc_uint<7> ux = 2;
        t0 = ux;
        z = -x + i - 1;
        t0 = z;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -2);  
        bz = (-x) * 3 + (-z) * i;
        t0 = bz.to_int();
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == 2);  
    }
  
    sc_signal<int> t1;
    void ternary() 
    {
        int z;
        sc_bigint<33> bz;
        int i = -5;
        unsigned uu = 7;
        t1 = uu;
        sc_int<7>  x = -4;
        sc_uint<7> ux = 2;
        t1 = ux;
        
        z = true ? 1 : 2;
        CHECK(z == 1);  
        t1 = z;
        z = false ? i : i - 1;
        t1 = z;
        CHECK(z == -6);  
        z = true ? (ux + i) * x : - 1;              // Warning reported
        t1 = z;
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 12);  
        z = true ? ((-x) * 3 + (-z) * i) : - 1;
        t1 = z;
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 72);  
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


