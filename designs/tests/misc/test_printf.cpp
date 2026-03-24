/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include <systemc.h>

using namespace sct;

// Check @printf generated to $display
struct A : public sc_module 
{
    sc_signal<int>  s{"s"};
    
    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(printfProc); 
        sensitive << s;
    }

    sc_signal<int> t2;
    void printfProc() {
        int i = 42;
        sc_uint<16> j = 43;
        sc_biguint<65> bj = 44;
        printf("i = %d\n", i);
        printf("i = %d, j = %d, bj = %d\n", i, j.to_uint64(), bj.to_uint64());
        //printf("j = %s\n", j.to_string().c_str()); -- not supported for synthesis
        cout << "i = " << i << ", j = " << j << ", bj = " << bj << endl;
        t2 = i + j.to_int() + bj.to_int();
    }
};


int sc_main(int argc, char **argv) 
{
    A top_mod{"top_mod"};
    sc_start();

    return 0;
}


