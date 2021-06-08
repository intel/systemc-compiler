/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

// Check unused variables/statements leads to member variables removed in SV
struct A : public sc_module, sc_interface
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    static const unsigned PORTS_NUM = 2;
    sc_vector<sc_signal<bool>> mw_renbl{"mw_renbl", PORTS_NUM};
    sc_vector<sc_signal<bool>> mw_rdata_valid{"mw_rdata_valid", PORTS_NUM};

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_CTHREAD(dvalidProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    void dvalidProc()
    {
        for (unsigned p = 0; p < PORTS_NUM; ++p) mw_rdata_valid[p] = 0;
        wait();

        while (true) {
            for (unsigned p = 0; p < PORTS_NUM; ++p)
                mw_rdata_valid[p] = mw_renbl[p];
            wait();
        }
    }        
};

struct B : public sc_module
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    A*  a[3];
    
    B(const sc_module_name& name) : sc_module(name) 
    {
        for (int i = 0; i < 3; ++i) {
            a[i] = new A("a");
            a[i]->clk(clk);
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    B b_mod{"b_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

