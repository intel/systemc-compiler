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
        SC_METHOD(binary); sensitive << s;
        SC_METHOD(binary_liter); sensitive << s;
        SC_METHOD(compound); sensitive << s;
        SC_METHOD(unary); sensitive << s;
        SC_METHOD(ternary); sensitive << s;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

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
    
    void unary() 
    {
        int z;
        sc_bigint<33> bz;
        int i = -5;
        unsigned uu = 7;

        sc_int<7>  x = -4;
        sc_uint<7> ux = 2;
        z = -x + i - 1;
        std::cout << "z = " << z << std::endl;
        CHECK(z == -2);  
        bz = (-x) * 3 + (-z) * i;
        std::cout << "z = " << bz << std::endl;
        CHECK(bz == 2);  
    }
    
    void ternary() 
    {
        int z;
        sc_bigint<33> bz;
        int i = -5;
        unsigned uu = 7;
        sc_int<7>  x = -4;
        sc_uint<7> ux = 2;
        
        z = true ? 1 : 2;
        CHECK(z == 1);  
        z = false ? i : i - 1;
        CHECK(z == -6);  
        z = true ? (ux + i) * x : - 1;              // Warning reported
        std::cout << "z = " << z << std::endl;
        //CHECK(z == 12);  
        z = true ? ((-x) * 3 + (-z) * i) : - 1;
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

