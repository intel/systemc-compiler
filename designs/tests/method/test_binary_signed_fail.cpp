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

// Errors detected for signed/unsigned casts
class A : public sc_module 
{
public:
    sc_in<bool>             a{"a"};
    sc_out<bool>            b{"b"};
    sc_signal<sc_uint<32>>  s{"s"};
    
    SC_CTOR(A)
    {
        SC_METHOD(liter_neg_div); sensitive << s;
        SC_METHOD(mixed_div); sensitive << s;
        SC_METHOD(mixed_non_div); sensitive << s;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    // Error negative literal casted to unsigned
    void liter_neg_div() 
    {
        sc_int<3> x = 2;
        sc_int<4> y = -6;
        sc_int<8> z;
        
        z = sc_int<4>(-6) / sc_uint<4>(2);          // Warning
        std::cout << "z = " << z << std::endl;
    }

    // Warning signed/unsigned mix division
    void mixed_div() 
    {
        sc_uint<3> x = 12;
        sc_int<4> y = -6;
        sc_int<8> z;
        
        z = x / y;                                  // Warning
        std::cout << "z = " << z << std::endl;
        z = int(x) / y;                            
        z = sc_int<4>(x) / y;                            
        
        unsigned uu = 24;
        z = uu / y;             
        std::cout << "z = " << z << std::endl;
        z = int(uu) / y;             
        z = sc_bigint<33>(uu) / y;             
        
        int i = -6;
        z = uu % i;                                 // Warning
        std::cout << "z = " << z << std::endl;

        z = x / i;                                  // Warning
        std::cout << "z = " << z << std::endl;
    }
    
    // Warning signed/unsigned mix non-division
    void mixed_non_div() 
    {
        sc_uint<3> x = 12;
        sc_int<4> y = -6;
        sc_int<8> z;
        
        z = x + y;                                  // Warning
        std::cout << "z = " << z << std::endl;
        z = sc_int<4>(x) + y; 
        
        unsigned uu = 24;
        z = uu - y;                                 
        std::cout << "z = " << z << std::endl;
        z = sc_int<16>(uu) - y;                                 
        
        int i = -6;
        z = uu * i;                                 // Warning
        std::cout << "z = " << z << std::endl;
        z = sc_bigint<24>(uu) * i;  

        z = x + i;                                  // Warning
        std::cout << "z = " << z << std::endl;
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

