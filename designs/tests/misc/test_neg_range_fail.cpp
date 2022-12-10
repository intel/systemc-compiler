/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Negative range/bit index, or high index less than low -- error reported
SC_MODULE(MyModule) 
{
    sc_signal<sc_uint<8>>  s;

    SC_CTOR(MyModule) {
        SC_METHOD(methodProc);
        sensitive << s;
    }   
    
    void methodProc() {
    	sc_uint<1> b = s.read().range(-1, 0); 
    	sc_uint<1> c = s.read().range(1, 3);
        bool d = s.read().bit(-1);
    }
};


int sc_main(int argc, char **argv) 
{
    MyModule top_mod{"top_mod"};
    sc_start();

    return 0;
}


