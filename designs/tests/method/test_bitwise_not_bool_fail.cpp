/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"


// Bitwise not for boolean, error reported
class A : public sc_module 
{
public:
    sc_signal<bool>     s{"s"};

    SC_CTOR(A)
    {
        SC_METHOD(test_bool_bitwise); 
        sensitive << s;
    }

    void test_bool_bitwise() {
        bool b = s.read();
        b = ~b;
    }
};


int sc_main(int argc, char* argv[])
{
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

