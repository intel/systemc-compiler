/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "test_func_in_cpp.h"

// Function implementation in cpp file, for #293

void Top::f1 (sc_uint<4>& par) {
    par = s.read();
}


int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    sc_signal<bool> rst{"rst"};
    
    Top top{"top"};
    
    top.clk(clock_gen);
    top.rst(rst);

    sc_start();

    return 0;
}

