/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Incorrect SC range, test fails
class A : public sc_module
{
public:
    sc_signal<bool>     a{"a"};
    sc_uint<4>          x;
    
    SC_CTOR(A)
    {
        SC_METHOD(extraRange); sensitive << a;
    }
    
    // Range wider than variable
    void extraRange() 
    {
        sc_uint<4> y = 3;
        x = y.range(4,1);
    }
};

class B_top : public sc_module
{

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

