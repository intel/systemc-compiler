/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 06/14/19.
//

#include "sct_comb_signal.h"
#include <systemc.h>

// Local variable modified in reset of thread in MIF array
struct mif : public sc_module, public sc_interface
{
    sc_in_clk           clk;
    sc_in<bool>         nrst;

    sc_signal<sc_uint<4>>     s;
    
    SC_HAS_PROCESS(mif);
    
    explicit mif(const sc_module_name& name) : sc_module(name)
    {
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    void threadProc() 
    {
        bool a = true;
        int b = 42;
        sc_uint<4> c = b >> 4;
        
        s = a ? (c + 1) : b; 
        wait();
        
        while (true) {
            s = s.read()+b;
            wait();
        }
    }
};

SC_MODULE(Top) 
{
    static const unsigned N = 2;
    
    sc_in_clk               clk{"clk"};
    sc_signal<bool>         nrst;
    
    mif*           minst[N];

    SC_CTOR(Top) 
    {
        for (int i = 0; i < N; i++) {
            minst[i] = new mif("mif");
            minst[i]->clk(clk);
            minst[i]->nrst(nrst);
        }
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
