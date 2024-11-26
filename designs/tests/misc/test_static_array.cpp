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

// Static member and static member array in module and in MIF array
struct M : public sc_module, sc_interface 
{
    static unsigned I;
    static unsigned J[2];
    static unsigned K[2][3];
    unsigned JJ[2];
    
    M(const sc_module_name& name) : sc_module(name) {}
};    

unsigned M::I;
unsigned M::J[2];
unsigned M::K[2][3];

struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;

    static unsigned E;
    static unsigned D[2];
    unsigned DD[2];

    sc_vector<M>  mif{"mif", 3};
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD(mif_method); sensitive << s;

        SC_CTHREAD(mif_thrd, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> t1;
    void mif_method() 
    {
        int l;
        l = mif[0].I;
        l = mif[s.read()+1].I;
        l = mif[0].JJ[0];
        l = mif[0].J[s.read()];
        mif[s.read()].I = mif[s.read()].J[0];
        l = D[s.read()];
        t1 = l;
    }
    
    sc_signal<int> t2;
    void mif_thrd() {
        unsigned l;
        l = mif[0].K[0][1] + E;
        t2 = l;
        wait();
        while(true) {
            t2 = mif[0].K[s.read()+1][s.read()-1] + DD[s.read()];
            wait();
        }
    }
};

unsigned A::E;
unsigned A::D[2];

int sc_main(int argc, char *argv[]) 
{
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

