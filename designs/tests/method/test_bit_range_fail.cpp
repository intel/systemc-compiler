/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// RValue expression in left/right part of assignment and in bit/range select
class A : public sc_module {
public:
    // Define variables
    sc_signal<bool> dummy;
    sc_int<8> x;
    sc_bigint<8> y;
    sc_uint<8> ux;
    sc_biguint<8> uy;

    SC_CTOR(A) {
        SC_METHOD(bit_range_sel); 
        sensitive << dummy;

        SC_METHOD(bit_range_sel2); 
        sensitive << dummy;
    }

    // RValue  used in left part of assignment, that is incorrect
    void bit_range_sel() {
        (y+y) = 0;
        (y+y)[3] = 0;
        (y+y)(5,3) = 0;
        (uy+uy)[3] = 0;
        (uy+uy)(5,3) = 0;
    }
    
    // RValue used for range/nit that is incorrect
    void bit_range_sel2() {
        sc_uint<8> a;
        sc_uint<8> b, c;
        
        // OK
        a = (((sc_uint<8>)(sc_uint<4>(b)))).bit(3);
        a = (((sc_uint<8>)(sc_uint<4>(b)))).range(2,1);
        
        // Not supported in VCS yet, {b,c}[2:1] -- no error reported
        a = ((sc_uint<8>)(b,c)).bit(3);
        a = ((sc_uint<8>)(b,c)).range(2,1);
        a = ((sc_uint<8>)(b,(sc_uint<4>)c)).range(2,1);
        a = ((sc_uint<8>)((sc_uint<6>)b,(sc_uint<4>)c)).range(2,1);

        // Error in VCS
        a = ((sc_uint<8>)(b++)).bit(3); 
        a = (((sc_uint<8>)(-b))).range(2,1); 
        a = ((sc_uint<8>)(b+c)).range(2,1); 
        
        a = ((sc_uint<8>)42).bit(4);
        a = ((sc_uint<8>)42).range(3,0);
        a = ((sc_uint<8>)(-42)).range(3,0);
        a = ((sc_uint<8>)(-42-1)).range(3,0);
    }
};

class B_top : public sc_module {
public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

