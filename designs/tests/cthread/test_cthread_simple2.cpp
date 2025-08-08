/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Simple clocked threads, including both clock edges and negative edge
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<bool>     c{"c"};
    sc_signal<bool>     d{"d"};

    SC_CTOR(A)
    {
        SC_CTHREAD(latch_issue, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_METHOD(latch_issue_meth); sensitive << ss;
        
        SC_CTHREAD(read_only_reg, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(simple1, clk.pos());
        async_reset_signal_is(nrst, true);
        
        SC_CTHREAD(simple2, clk.neg());
        async_reset_signal_is(nrst, false);

        // clk posedge generated
        SC_CTHREAD(simple3, clk);
        async_reset_signal_is(nrst, false);
        
        SC_CTHREAD(simple_pres, clk.pos());
        async_reset_signal_is(nrst, false);
        
    }

    // Issue with no initialization for @par -- FIXED
    sc_signal<int>  ss{"ss"};
    sc_signal<int>  tt;
    
    void f(uint32_t par) {
        tt = par;
    }
    
    void latch_issue() {
        tt = 0;
        f(0);
        wait();
        
        while(true) {
            int loc;
            loc = ss.read();
            f(loc);
            wait();
            tt = 0;
            wait();              
        }
    }
    
    
    sc_signal<int>  tt1;
    
    void f1(uint32_t par) {
        tt1 = par;
    }

    void latch_issue_meth() {
        tt1 = 0;
        if (ss.read()) {
            f1(ss.read());
        } else {
            f1(ss.read());
        }
    }
    
//-----------------------------------------------------------------------------
    
    int                 m;
    int                 m1;
    int                 k;
    int                 n;

    sc_signal<int> t0;
    void read_only_reg()
    {
        wait();
        
        while (true) {
            bool b = c;
            t0 = b;
            wait();
        }
    }
    
    void simple1()
    {
        m = 1;
        int i = 0;
        wait();
        
        while (true) {
            i = m + 1;
            m = i;
            wait();
        }
    }
    
    void simple2()
    {
        int x = 1;
        wait();
        
        while (true) {
            int i = x + 1;
            x = i;
            wait();
        }
    }
    
    void simple3()
    {
        b = 1;
        wait();
        
        while (true) {
            c = a + b;
            wait();
            b = c;
            wait();
        }
    }

    sc_signal<bool>     e;
    void simple_pres()
    {
        e = 0;
        int i = 1;
        wait();
        
        while (true) {
            e = a + i;
            wait();
            i = e;
            wait();
        }
    }
    
    void simple1_()
    {
        m1 = a.read();
    }

};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

