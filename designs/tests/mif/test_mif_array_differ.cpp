/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Test for constant analysis for MIF array elements, check MIF elements
// differ by various constructor parameters
class MIF : public sc_module, sc_interface 
{
public:
    sc_signal<int>  t;
    sc_in<int>  p;
    
    unsigned M;   // Considered as constant if same in all MIF array elements  
    const unsigned AR[2];  // Is not considered as constant  

    SC_HAS_PROCESS(MIF);
    MIF(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name), M(par), 
#ifdef OPT3
        AR{par, par+1}
#else
        AR{par, par}
#endif
    {
        SC_METHOD(mif_proc); sensitive << t;
    }
    
    sc_signal<int>  tt;
    void mif_proc() {
        tt = AR[0];
        if (M == 42) tt = AR[1];
        if (AR[0] == 2) tt = 0;
    }
};

// Array/vector of MIF with different variables used in one/multiple local process
class A : public sc_module, sc_interface 
{
public:
    sc_signal<int>  s;

    MIF     mif1;       // @M has same values and considered as constant
    MIF     mif2;       // @M has different values 
    sc_vector<MIF>     mif_arr;
    
    SC_HAS_PROCESS(A);
    A(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name), C(par), BR{1,1}
        ,mif1{"mif1", par}, mif2{"mif2", par+1} 
    {
#ifdef OPT2
        mif_arr.init(2, [=](const char* name, size_t i) {
                       return new MIF(name, par+i);});
#else
        mif_arr.init(2, [=](const char* name, size_t i) {
                       return new MIF(name, par);});
#endif
        
        SC_METHOD(cpa_issue); sensitive << s;
        SC_METHOD(proc1); sensitive << s;
        SC_METHOD(proc2); sensitive << s;
        V = par; X = 42;
        Z = par == 1;
        
        mif1.p(s);
        mif2.p(s);
        mif_arr[0].p(s); mif_arr[1].p(s);
    }
    
    sc_uint<16> X;    // Used in 2 processes
    int V;
    const unsigned W = 42;
    bool Z;
    const unsigned C;
    const int BR[2];
    
    sc_signal<int> t2;
    void cpa_issue() 
    {
        t2 = 0;
        if (W) t2 = 1; 
        if (C) t2 = 2; 
        if (X) t2 = 3; 
        if (BR[s.read()]) t2 = 1; 
        if (BR[0]) t2 = 0;
    }
    
    sc_signal<int> t0;
    void proc1() 
    {
        unsigned l = X;
        if (C) l = V;              // Problem in generate block!!! 
        l = (C == 1) ? 42 : W+1;
        l = Z ? 1 : 2;
        t0 = l;
    }
    
    sc_signal<int> t1;
    void proc2() 
    {
        unsigned l;
        l = X+1;
        t1 = l;
    }
};

SC_MODULE(Top) 
{
    A   a1{"a1", 42};
    A   a2{"a2", 43};
#ifdef OPT1    
    sc_vector<A> ar{"ar", 2,
                    [](const char* name, size_t i) 
                    {return new A( name, i );}};
#else 
    sc_vector<A> ar{"ar", 2,
                    [](const char* name, size_t i) 
                    {return new A( name, 1 );}};
#endif
    
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

