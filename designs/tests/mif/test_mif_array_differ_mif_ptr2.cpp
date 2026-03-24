/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// MIF array elements with MIF array elements with MIF pointers as 
// internal @sct_fifo pointer in @sct_target

class MIF : public sc_module, sc_interface 
{
public:
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    const int C;

    SC_HAS_PROCESS(MIF);
    MIF(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name), C(par)
    {
        SC_CTHREAD(mif_proc, clk.pos()); 
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int>  t;
    sc_signal<int>  s;
    sc_vector<sc_signal<int>>  vec{"vec", 2};
    
    void mif_proc() {
        t = 0;
        vec[0] = 0; vec[1] = 1;
        wait();
        while (true) {
            for (int i = 0; i != 2; ++i) {
                vec[i] = t.read();
            }
            t = vec[s.read()].read() + C;
            wait();
        }
    }
};

class A : public sc_module, sc_interface 
{
public:
    sc_in_clk       clk;
    sc_signal<int>  s;

    MIF*            mif;       
    
    SC_HAS_PROCESS(A);
    A(const sc_module_name& name, unsigned par = 0) : 
        sc_module(name)
        
    {
        mif = new MIF("mif", par);
        mif->clk(clk);
        
        SC_METHOD(proc); sensitive << s << mif->s << mif->vec[0] << mif->vec[1];
    }
    
    sc_signal<int> t0;
    void proc() 
    {
        mif->s = 42;
        t0 = mif->vec[mif->s.read()];
    }
};

class B : public sc_module, sc_interface 
{
public:
    sc_in_clk       clk;

    sc_vector<A> ar{"ar", 3,
                    [](const char* name, size_t i) 
                    {return new A( name, 44+i );}};
    
    B(const sc_module_name& name) : 
        sc_module(name)
        
    {
        ar[0].clk(clk); ar[1].clk(clk); ar[2].clk(clk);
    }
};

SC_MODULE(Top) 
{
    sc_in_clk       clk;
    sc_vector<B> br{"br", 2};
    
    SC_CTOR(Top) {
        br[0].clk(clk); br[1].clk(clk);
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();
    return 0;
}

