/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Temporary expression in LHS, errors reported
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<sc_uint<2> >  sig1;
    sc_signal<sc_uint<3> >  sig_arr[3];
 
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(sc_variable_cast); 
        sensitive << dummy;
        
        SC_METHOD(sc_channel_cast); 
        sensitive << dummy;
    }

    // type cast to get bit/range of argument
    void sc_variable_cast() {
        sc_uint<2> x;
        sc_uint<4> y; 

        // Cast in assignment
        (sc_uint<2>)y = x;
        static_cast<sc_uint<2> >(y) = x;
    }   

    // type cast for ports/signals
    void sc_channel_cast() {
        sc_uint<2> x;
        (sc_uint<2>)sig1.read() = x;        // Stupid code, just to check not fail
        (sc_uint<2>)sig_arr[1].read() = x;  // Stupid code, just to check not fail
    }    
};

class B_top : public sc_module {
public:
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

