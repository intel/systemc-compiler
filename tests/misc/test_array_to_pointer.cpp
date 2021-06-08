/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Array of non-channel pointers, including 2D array access via pointer
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    int*          pi;
    int*          pj;
    int           pa[3];
    int           paa[3];
    int*          pb[3];
    int           pc[3];
    int*          pd[3];

    int*          pp;
    int*          pp1;
    
    int*          parr[3][4];
    
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
            pc[i] = i;
            *pb[i] = i;
            *pd[i] = i;
            pi[i] = i;
            pj[i] = i;
            for (int j = 0; j < 4; j++) {
                parr[i][j] = sc_new<int>();
            }
        }
        
        // Pointer to global array
        pp = paa;
        // Assignment pointer to another pointer
        int* qq = sc_new<int>();  
        pp1 = qq;
        
        SC_METHOD(read_pointer); 
        sensitive << s;

        SC_METHOD(read_array); 
        sensitive << s;
        
        SC_CTHREAD(write_array, clk.neg()); 
        async_reset_signal_is(nrst, 0);
    }

    void read_pointer()
    {
        int larr[3];
        int *lp = larr;
        int i;
        i = *pp1;
        i = pp[1];
        i = lp[1];
    }

    
    int arr[3][4];
    void read_array()
    {
        int i;
        i = pi[0];
        i = pa[1];
        i = *pb[s.read()];
        
        int* lp1 = arr[0];
        lp1[1] = 42;
        sct_assert_const(arr[0][1] == 42);
        
        int* lp2 = arr[s.read()];
        i = lp2[s.read()+1] + lp1[2];
        
        *parr[2][1] = 43;
        sct_assert_const(*parr[2][1] == 43);
        
        int* lp3 = parr[s.read()][1];
        *lp3 = 44;
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
 
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk("clk", 1, SC_NS);
    A a_mod{"b_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}

