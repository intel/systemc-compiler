/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Test for constant analysis for MIF array elements with non-constant which
// should become @localparam in generated SV

class MIF : public sc_module, sc_interface 
{
public:
    const int c1 = 44;
    const int c2;
    int m1 = 40;
    int m2;

    SC_HAS_PROCESS(MIF);
    MIF(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name), c2(par), m2(par+1)
    {
        SC_METHOD(mif_proc); sensitive << t;
    }
    
    sc_signal<int>  t;
    sc_signal<int>  tt;
    void mif_proc() {
        if (c1 == 44) m1 = t;               // Condition evaluated, no @if generated
        if (c2 == 44) m1 = t.read() + 1;    // Condition not evaluated
        tt = m1;
    }
};


class B : public sc_module, sc_interface 
{
public:
    sc_signal<int>  s;

    int b1 = 40;
    int b2;
    
    SC_HAS_PROCESS(B);
    B(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name), b2(par)
    {}
};

class A : public B
{
public:
    sc_signal<int>  s;

    MIF             mif1;       
    sc_vector<MIF>  mif_arr;
    
    int v1 = 42;
    int v2;
    
    SC_HAS_PROCESS(A);
    A(const sc_module_name& name, unsigned par = 0) : 
        B(name, par-1), v2(par+1), mif1{"mif1", par}
    {
        mif_arr.init(2, [=](const char* name, size_t i) {
                      return new MIF(name, par+i);});
        
        SC_METHOD(proc); sensitive << s;
    }
    
    sc_signal<int> t1;
    void proc() 
    {
        unsigned l = b1 + b2 + v1 + v2;
        l = mif1.m2;
        t1 = l;
    }
};

SC_MODULE(Top) 
{
    sc_vector<A> ar{"ar", 2,
                    [](const char* name, size_t i) 
                    {return new A( name, 44+i );}};
    
    SC_CTOR(Top) {
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top{"top"};
    
    sc_start();
    return 0;
}

