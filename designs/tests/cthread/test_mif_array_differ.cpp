/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Array/vector of module with different values set to variables at elaboration
class A : public sc_module, sc_interface
{
public:
    sc_in<bool> clk;
    sc_signal<bool>  nrst;
    sc_signal<bool>  s;

    SC_HAS_PROCESS(A);
    A(const sc_module_name& name, unsigned par = 0) : sc_module(name), C(par) 
    {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(nrst, false);
        V = par;
    }
        
   
    unsigned V;
    const unsigned C;
    
    void proc() 
    {
        unsigned l = V;
        wait();
        while (true) {
            l = V + C;
            wait();
        }
    }
};

class B : public sc_module, sc_interface
{
public:
    sc_in<bool> clk;
    sc_signal<bool>  nrst;

    SC_HAS_PROCESS(B);
    B(const sc_module_name& name, unsigned par = 0) : sc_module(name)
    {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(nrst, false);
        V = par;
    }
        
    unsigned V;
    
    void proc() 
    {
        wait();
        while (true) {
            unsigned l = V;
            wait();
            l = V + 1;
        }
    }
};

SC_MODULE(Top) 
{
    sc_in<bool> clk;
    A*  ar1[2];
    B*  br1[2];
    
    sc_vector<A> ar2{"ar2", 2, 
                     [](const char* name, size_t i) 
                     {return new A( name, i+1 );}};
    sc_vector<B> br2{"br2", 2, 
                     [](const char* name, size_t i) 
                     {return new B( name, i+1 );}};
    
    sc_vector<A> ar3{"ar3", 2};
    sc_vector<B> br3{"br3", 2};

    SC_CTOR(Top) {
        for (int i = 0; i < 2; ++i) {
            ar1[i] = new A("ar1", i+1);
            ar1[i]->V = i+1;
            br1[i] = new B("br1", i+1);
            br1[i]->V = i+1;
            ar3[i].V = i+1;
            br3[i].V = i+1;

            ar1[i]->clk(clk);
            ar2[i].clk(clk);
            ar3[i].clk(clk);
            br1[i]->clk(clk);
            br2[i].clk(clk);
            br3[i].clk(clk);
        }
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

