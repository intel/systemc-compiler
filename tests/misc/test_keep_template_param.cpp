/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Generate SV parameter for top module template parameter, #299
template <int N>
struct A : public sc_module
{
    sc_in<bool>     clk;
    sc_signal<int>  s;
    
    static const int C = 3;
    static const int D = C + N;
    
    sc_signal<sc_uint<C>> c{"c"};
    sc_signal<sc_uint<D>> d{"d"};
    sc_signal<sc_uint<N>> n{"n"};
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(methProc);
        sensitive << s;
    }
   
    void methProc()
    {
        int i = N + C + D;
        s = i;
        c = i; d = i; n = i;
    }
};

//-----------------------------------------------------------------------------

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A<4> a{"a"};
    a.clk(clk);
    
    sc_start();
    return 0;
}

