/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Array/vector of MIF with different variables used in one/multiple local process
class A : public sc_module, sc_interface 
{
public:
    sc_signal<int>  s;

    SC_HAS_PROCESS(A);
    A(const sc_module_name& name, unsigned par = 0) : sc_module(name), C(par) 
    {
        SC_METHOD(proc); sensitive << s;
        SC_METHOD(proc2); sensitive << s;
        V = par; W = par; X = par+42;
        Z = par == 1;
    }
    
    sc_uint<16> X;    // Used in 2 processes
    int V;
    unsigned W;
    bool Z;
    const unsigned C;
    
    void proc() 
    {
        unsigned l = X;
        if (C) l = V;
        l = (C == 1) ? 42 : W+1;
        l = Z ? 1 : 2;
    }
    
    void proc2() 
    {
        unsigned l;
        l = X+1;
    }
};

SC_MODULE(Top) 
{
    sc_vector<A> ar{"ar", 2,
                    [](const char* name, size_t i) 
                    {return new A( name, i );}};
    
    SC_CTOR(Top) {
        for (int i = 0; i < 2; ++i) {
            ar[i].V = i+1;
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top{"top"};
    
    sc_start();
    return 0;
}

