/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Array of pointers with some pointers not initialized, #166
class A : public sc_module
{
public:
    sc_signal<sc_uint<4>>* s[3] = {};

    SC_CTOR(A)
    {
        // Array of dynamically allocated integers
        for (int i = 0; i < 3; i++) {
            // if (i != 0) {  -- assert in ScElabModuleBuilder::traverseArray()
            if (i != 1) { // -- incorrect code generated, #166
                s[i] = new sc_signal<sc_uint<4>>("s");
            }
        }
        
        SC_METHOD(methProc); 
        for (int i = 0; i < 3; i++) {
            if (s[i]) sensitive << *s[i];
        }
    }

    void methProc()
    {
        bool l = 0;
        for (int i = 0; i < 3; i++) {
           if (s[i]) {
               l = l || s[i]->read();
           }
        }
    }
};

int sc_main(int argc, char* argv[])
{
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

