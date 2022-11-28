/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Function return from loop, test fails
SC_MODULE(test) {

    sc_signal<bool> in{"in"};
    sc_signal<bool> out{"out"};
    sc_signal<sc_uint<3>> a{"a"};

    SC_CTOR(test) {
        SC_METHOD(return_from_loop);
        sensitive << in;
    }

    // Conditional @return from loop
    // Incorrect code generated now as code after return is not supported
    bool f2() 
    {
        for (int i = 0; i < 3; i++) {
            if (in) return true;
        }
        return false;        
    }
    
    void return_from_loop() {
        bool b = f2();
    }

};

int sc_main(int argc, char **argv) {
    test t_inst{"t_inst"};
    sc_start();
    return 0;
}

