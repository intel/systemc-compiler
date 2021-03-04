/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Array of non-channel pointers
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    int*          pi;
    int*          pj;
    int           pa[3];
    int*          pb[3];
    int           pc[3];
    int*          pd[3];
    int*          pp;
    int*          ppp; 
    int*          pq;
    
    sc_signal<int> s;

    SC_CTOR(A)
    {
        // Dynamically allocated array stored into pointer
        pi = sc_new_array<int>(3);
        pj = sc_new_array<int>(3);
        
        // Array of dynamically allocated integers
        for (int i = 0; i < 3; i++) {
            pb[i] = sc_new<int>();
            pd[i] = sc_new<int>();
            pa[i] = i;
            pc[i] = i;
            *pb[i] = i;
            *pd[i] = i;
            pi[i] = i;
            pj[i] = i;
        }
        
        // TODO: Fix me, #102
        /*pp = &pa[1];
        ppp = pp+1;
        pq = pb[1];*/
        
        SC_METHOD(read_array); 
        sensitive << s;
        
        SC_CTHREAD(write_array, clk.neg()); 
        async_reset_signal_is(nrst, 0);
        
        // TODO: Fix me, #102
        //SC_METHOD(read_pointer); 
        //sensitive << s;
    }

    void read_array()
    {
        int i;
        i = pi[0];
        i = pa[1];
        i = *pb[s.read()];
    }
    
    void write_array() 
    {
        for (int i = 0; i < 3; i++) {
            pj[i] = i;
        }
        wait();
        
        while (1) {
            pc[s.read()] = pj[1];
            wait();
            *pd[s.read()+1] = pc[1];
        }
    }
    
    void read_pointer()
    {
        int i;
        i = *pp;
        i = *ppp;
        i = *pq;
    }
 
};

class B_top : public sc_module
{
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

