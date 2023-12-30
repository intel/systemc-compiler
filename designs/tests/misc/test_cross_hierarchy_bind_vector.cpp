/******************************************************************************
* Copyright (c) 2023, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include <systemc.h>

// Cross-hierarchy bind of vector of signals/ports
template< class T>
struct A : public sc_module 
{
    sc_in<bool>     clk{"clk"};
    sc_in<bool>     nrst{"nrst"};

    sc_vector<sct_in<T>>    SC_NAMED(ains, 2);     
    sc_vector<sct_signal<T>>    SC_NAMED(asigs, 2);
    
    SC_CTOR(A) {
        SC_METHOD(proc);
        sensitive << ains[0] << ains[1];
    }
    
    void proc() {
        asigs[0] = ains[0].read();
        asigs[1] = ains[1].read();
    }
};

template< class T>
struct B : public sc_module 
{
    sc_in<bool>     clk{"clk"};
    sc_in<bool>     nrst{"nrst"};

    sc_vector<sct_in<T>>    SC_NAMED(bins, 2);     
    
    A<T>      SC_NAMED(mod_a);
    
    SC_CTOR(B) {
        mod_a.clk(clk); mod_a.nrst(nrst);
        
        bins(mod_a.asigs);
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<T>        s;
    void threadProc() {
        unsigned cntr = 0;
        wait();
        while (true) {
            cntr += bins[0].read() + bins[1].read();
            s = cntr;
            wait();
        }
    }
};

struct Top : sc_module {

    using T = sct_uint<12>;
    
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    sc_signal<T>        s;
    
    sc_vector<sct_signal<T>>    SC_NAMED(sigs, 2);

    B<T>       SC_NAMED(mod_b);
    
    SC_CTOR(Top) 
    {
        mod_b.clk(clk); mod_b.nrst(nrst);
        mod_b.mod_a.ains[0].bind(sigs[0]);
        mod_b.mod_a.ains[1].bind(sigs[1]);

        SC_METHOD(proc);
        sensitive << s;
    }
    
    void proc() {
        sigs[0] = 0; sigs[1] = s.read();
    }
};


int sc_main(int argc, char** argv)
{
    sc_clock clk("clk", 1 , SC_NS);
    Top top("top");
    top.clk(clk);
    
    sc_start();
    return 0;
}

