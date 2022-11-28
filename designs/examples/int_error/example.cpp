/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "systemc.h"
 
// Internal error reporting
struct Dut : sc_module 
{
    // Dangling pointer
    sc_uint<4>* x;
    sc_signal<bool> s;
 
    SC_CTOR(Dut) 
    {
	SC_METHOD(methodProc);
	sensitive << s;
    }

    void methodProc() {
        if (x) {
            int i = 42;
        }
    }

};

int sc_main (int argc, char **argv) 
{
    Dut dut("dut");
    sc_start();
    return 0;
}