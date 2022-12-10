/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Large array test to improve @ProcBuilder performance
struct top : sc_module 
{
    static const unsigned ARR_SIZE = 4*1024;
    sc_signal<sc_uint<8>>  arr[ARR_SIZE];
    sc_signal<sc_uint<8>>  out;
    
    sc_signal<bool> s;

    SC_CTOR(top) {
        SC_METHOD(proc);
        sensitive << arr[0];
    }
    
    void proc() {
        out = arr[0];
    }
    
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

