/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Array of global(non-local) record with functions (methods) called in CTHREAD
// Additionally check pointer to record and record pointer array
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    sc_signal<int> sig;

    SC_CTOR(A) 
    {
        sp = sc_new<Simple>();
        rp = sc_new<Simple>();
        
        for (int i = 0; i < 2; i++) {
            w[i] = sc_new<Simple>();
            z[i] = sc_new<Simple>();
            g[i] = sc_new<Simple>();
            f[i] = sc_new<Simple>();
        }
        
        SC_CTHREAD(single_rec_call_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(single_rec_call_comb, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(pointer_rec_call_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(pointer_rec_call_comb, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_reg, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(rec_arr_call_comb, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(pointer_arr_call_reg, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(pointer_arr_call_comb, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(pointer_arr_call_unknw_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(pointer_arr_call_unknw_reg2, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    struct Simple {
        bool a;
        int b;
        
        void reset() {
            a = 0; b = 0;
        }
        
        void setA(bool par) {
            a = par;
        }
        
        void setB(int par) {
            b = par;
        }

        bool getA() {
            return a;
        }

        int getB() {
            return b;
        }
    };
    
    // Function called in single record, @s is register
    Simple s;
    void single_rec_call_reg() 
    {
        wait();
        
        while (true) {
            s.setA(1);
            wait();

            bool c = s.getA();
        }
    }

    // Function called in single record, @r is not register
    Simple r;
    void single_rec_call_comb() 
    {
        wait();
        
        while (true) {
            r.setA(1);
            bool c = r.getA();
            
            wait();
        }
    }

    // Function called in record pointer, @s is register
    Simple* sp;
    void pointer_rec_call_reg() 
    {
        wait();
        
        while (true) {
            sp->setA(1);
            wait();

            bool c = sp->getA();
        }
    }
    
    // Function called in record pointer, @r is not register
    Simple* rp;
    void pointer_rec_call_comb() 
    {
        wait();
        
        while (true) {
            rp->setA(1);
            bool c = rp->getA();
            
            wait();
        }
    }
    
// ----------------------------------------------------------------------------    
    // Function called in record array, @s is register
    Simple t[2];
    void rec_arr_call_reg() 
    {
        wait();
        
        while (true) {
            t[1].setA(1);
            wait();

            bool c = t[1].getA();
        }
    }

    // Function called in record array, @s is not register
    Simple q[2];
    void rec_arr_call_comb() 
    {
        wait();
        
        while (true) {
            q[1].setA(1);
            bool c = q[1].getA();
            wait();
        }
    }
    
    Simple* w[2];
    void pointer_arr_call_reg() 
    {
        wait();
        
        while (true) {
            w[1]->setA(1);
            wait();

            bool c = w[1]->getA();
        }
    }

    Simple* z[2];
    void pointer_arr_call_comb() 
    {
        wait();
        
        while (true) {
            z[1]->setA(1);
            bool c = z[1]->getA();
            wait();
        }
    }

// ----------------------------------------------------------------------------    
    // Function called in record array at unknown index, @s is register
    Simple* g[2];
    void pointer_arr_call_unknw_reg() 
    {
        wait();
        
        while (true) {
            int j = sig.read();

            g[1]->setA(1);
            bool c = g[j]->getA();

            wait();
        }
    }
    
    Simple* f[2];
    void pointer_arr_call_unknw_reg2() 
    {
        wait();
        
        while (true) {
            int j = sig.read();

            f[j]->setA(1);
            bool c = f[j]->getA();

            wait();
        }
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);

    sc_start();
    return 0;
}

