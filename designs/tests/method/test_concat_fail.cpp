/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <iostream>

using namespace sc_core;

// Concatenation with operand not specified length, error reported
class A : public sc_module {
public:
    sc_signal<bool> a;

    SC_CTOR(A) {
        SC_METHOD(icorrect_concat); 
        sensitive << a;

        SC_METHOD(concat_cpp); 
        sensitive << s3 << s4;

        SC_METHOD(cast_lhs); sensitive << s3;
    }
    
    void icorrect_concat() 
    {
        sc_uint<4> i,j;
        sc_uint<8> k;

        // "i*j" casted to boolean in Clang AST, therefore {i, |(i*j)} in Veirlog
        k = (i, i*j);                       // ERROR
        k = (i-j, j);                       // ERROR
    }
    
    sc_signal<int>          s3;
    sc_signal<unsigned>     s4;
    
    // Concat with non-intended comma for SC and CPP types mix
    void concat_cpp() 
    {
        sc_uint<1> y = s3.read();
        sc_uint<12> yy = s3.read();
        sc_biguint<33> by = s4.read();
        int i = s3.read();
        sc_uint<10> z;
        sc_biguint<40> bz;

        // Comma has no data width, error reported
        z = (s3, 0, y);                     // ERROR
        z = (yy*y, i);  
        bz = (by, yy + s4.read(), s3);      // ERROR
    }
    
    void cast_lhs() 
    {
        int k;
        sc_uint<4> a;
        unsigned i;

        // Error reported: LValueBitCast not supported
        (int&)a = s3.read();
        if (a) k = 1;
        (int&)i = s3.read();
        if (i) k = 1;
    }
};

class B_top : public sc_module {
public:
    A a_mod{"a_mod"};
    
    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) 
{
    B_top bmod{"b_mod"};
    sc_start();
    return 0;
}

