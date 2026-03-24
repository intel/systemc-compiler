/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// MIF array elements with records and MIF/MIF arrays with records inside, 
// check MIF elements differ by various constructor parameters
struct Simple {
    int v;
    const int c;
    
    Simple(int par) : c(par) {}
};

class MIF : public sc_module, sc_interface 
{
public:
    Simple      r;

    SC_HAS_PROCESS(MIF);
    MIF(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name), r(par)
    {
        SC_METHOD(mif_proc); sensitive << t;
    }
    
    sc_signal<int>  t;
    sc_signal<int>  tt;
    void mif_proc() {
        tt = r.c + r.v;
    }
};

class A : public sc_module, sc_interface 
{
public:
    sc_signal<int>  s;

    Simple          r1;
    Simple          rr[2];
    MIF             mif1;       
    sc_vector<MIF>  mif_arr;
    
    int vv;
    
    SC_HAS_PROCESS(A);
    A(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name), r1(par), vv(par), rr{par, par+1}
        ,mif1{"mif1", par}
    {
        mif_arr.init(2, [=](const char* name, size_t i) {
                       return new MIF(name, par+i+100);});
        
        SC_METHOD(proc1); sensitive << s;
    }
    
    sc_signal<int> t0;
    void proc1() 
    {
        unsigned l = r1.c + r1.v + vv;
        l = rr[s.read()].c + rr[s.read()].v;
        t0 = l;
    }
};

SC_MODULE(Top) 
{
    A   a1{"a1", 42};
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

