/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Function implementation in cpp file, for #293

struct Top : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    
    sc_signal<bool>     s;
    
    SC_HAS_PROCESS(Top);
    Top (const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(methProc);
        sensitive << s;
    }
    
    void f1 (sc_uint<4>& par);
    
    sc_signal<int>     t0;
    void methProc() {
        sc_uint<4> l;
        f1(l);
        t0 = l;
    }
};
