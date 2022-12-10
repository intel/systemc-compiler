/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Port bound to signal located in the same module, no port promoted outside
// warning generated (not error because such style used in MemoCat for CG)

struct A : sc_module {
    sc_in<int>   in{"in"};
    sc_out<int>  out{"out"};
    sc_signal<int> s;
    
    SC_CTOR(A) 
    {
        out(s);
        in(s);
    }
};

struct Top : sc_module 
{
    sc_signal<int>  in_s;
    sc_signal<int>  out_s;
    
    A a{"a"};
    
    SC_CTOR(Top) 
    {
        a.in(in_s);
        a.out(out_s);
    }
};

int sc_main(int argc, char** argv)
{
    Top top("top");
    
    sc_start();
    return 0;
}

