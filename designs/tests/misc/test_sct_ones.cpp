/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include <systemc.h>

using namespace sct;

// Check name generation for @sct_ones and @sct_zeros
template <unsigned N>
struct Top : public sc_module 
{
    sc_signal<sct_uint<N>>  s{"s"};
    
    Top(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(oncesProc); 
        sensitive << s;
    }

    using T = sct_uint<N>;
    const int A = -sct_ones<N>; 
    const sc_bigint<68> B = -sct_ones<65>;

    sc_signal<int> t1;
    void oncesProc() {
        t1 = A + B.to_int();
        t1 = sct_zeros<N> + sct_zeros<N+1>;
        t1 = sct_ones<N-1> + sct_ones<N>;
        cout << typeid(int).name() << typeid(t1).name() << endl;
    }

};


int sc_main(int argc, char **argv) 
{
    Top<16> top_mod{"top_mod"};
    sc_start();

    return 0;
}


