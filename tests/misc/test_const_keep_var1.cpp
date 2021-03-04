/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

// Check correct constant for instances of the same module type
template<unsigned N>
struct A : public sc_module, public sc_interface
{
    sc_signal<int> s;
    
    SC_HAS_PROCESS(A);

    static const unsigned BYTE_EN_WIDTH = 32 / N;
    
    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD(byteEnProc);
        sensitive << s;
    }
   
//-----------------------------------------------------------------------------
   
    // Bug incorrect BYTE_EN_WIDTH suffix -- fixed
    void byteEnProc()   
    {
        sc_uint<32> data = s.read();
        for (unsigned i = 0; i < BYTE_EN_WIDTH; ++i) {
            bool b = data.bit(N*i);
        }
    }
    
};

//-----------------------------------------------------------------------------

struct B : public sc_module{
    
    sc_in<bool>         clk{"clk"};
    A<2> a1{"a1"};
    A<4> a2{"a2"};
    A<4> a3{"a3"};
    A<2> a4{"a4"};
    
    explicit B(const sc_module_name& name)
    {}
};


int sc_main(int argc, char *argv[]) 
{
    B mod{"mod"};
    sc_clock clk{"clk", 1, SC_NS};
    mod.clk(clk);
    
    sc_start();
    return 0;
}

