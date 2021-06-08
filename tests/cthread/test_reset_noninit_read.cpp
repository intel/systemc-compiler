/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Read not initialized variables in reset
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    SC_CTOR(A)
    {
        SC_CTHREAD(readProc, clk.pos());
        async_reset_signal_is(nrst, false);
    }
    
    sc_signal<int>     s{"s"};
    
    void f1(int par_a[3]) {
        auto ii = par_a[1];
    }
    void f2(int (&par_a)[3], int i) {
        auto ii = par_a[i];
    }
    void f3(int par_a[3][3], int i) {
        auto ii = par_a[i+1][i+2];
    }

    void readProc() 
    {
        int aa[3];
        int bb[3] = {0,1,2};
        int cc[3][3];

        f1(aa);
        f2(bb, 1);
        f3(cc, 0);

        wait();
        
        while(1) {
            wait();
        }
    }
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk("clk", 1, SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}