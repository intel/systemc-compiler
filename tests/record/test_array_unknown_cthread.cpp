/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record arrays with unknown index access and record copy in CTHREAD 
template <unsigned N>
class A : public sc_module {
public:
    sc_in<bool>     clk;
    sc_signal<bool> nrst{"nrst"};
    sc_signal<sc_uint<4>> sig{"sig"};

    SC_CTOR(A) 
    {
          // #141
//        SC_CTHREAD(copy_rec_arr_elem, clk.pos());  
//        async_reset_signal_is(nrst, 0);
//
//        SC_CTHREAD(copy_rec_arr_elem2, clk.pos());  
//        async_reset_signal_is(nrst, 0);
//
//        SC_CTHREAD(copy_rec_arr_unknw, clk.pos());  
//        async_reset_signal_is(nrst, 0);
//
//        SC_CTHREAD(copy_rec_arr_unknw2, clk.pos());  
//        async_reset_signal_is(nrst, 0);
//        
//        SC_CTHREAD(copy_rec_arr_unknw3, clk.pos());  
//        async_reset_signal_is(nrst, 0);
//        
//        SC_CTHREAD(copy_rec_arr_mult, clk.pos());  
//        async_reset_signal_is(nrst, 0);
    }
   
    
    struct Simple {
        sc_int<2>  a;
        sc_uint<4> b;
    };
    
    
    // Copy record array element
    Simple gr[2];
    void copy_rec_arr_elem() 
    {
        int j = sig.read();
        wait(); 
        while (true) {
            Simple ar[3];   // @a is comb, @g is register
            
            ar[2] = gr[1];
            ar[j].a = 0;
            wait();
        }
    }
    
    Simple gp[2];
    void copy_rec_arr_elem2() 
    {
        int j = sig.read();
        wait(); 
        while (true) {
            Simple ap[3];   // @a is register, @g is comb
            wait();
            
            gp[1] = ap[2];
            gp[j].a = 0;
        }
    }
    
    // Copy at unknown index
    Simple gs[2];
    void copy_rec_arr_unknw() 
    {
        wait(); 
        while (true) {
            Simple as[3];   // @a is register, @g is comb
            wait();
            
            int j = sig.read();
            gs[j] = as[j];
            j = as[j].b;
        }
    }
 
    // Copy at unknown index
    Simple gt[2];
    void copy_rec_arr_unknw2() 
    {
        wait(); 
        while (true) {
            Simple at[3];   // @a is comb, @g is register
            wait();
            
            int j = sig.read();
            at[j] = gt[j];
            at[j].b = gt[j+1].a;
        }
    }
    
    
    Simple gu[2];
    void copy_rec_arr_unknw3() 
    {
        wait(); 
        while (true) {
            Simple au[3];   // @a is register, @g is register
            wait();
            
            int j = sig.read();
            au[j] = gu[j];
            gu[0] = au[j];
        }
    }
    
    // Multi-dimensional array
    Simple guu[2][3];
    void copy_rec_arr_mult() 
    {
        wait(); 
        while (true) {
            Simple auu[2][3];  
            wait();
            
            int j = sig.read();
            auu[j][0] = guu[1][j];
            auu[0][j+1] = guu[j-1][2];
            
            auu[0][j+1].a = j;
            j = guu[j][j+1].b;
        }
    }
    
};

class B_top : public sc_module {
public:
    sc_in<bool>     clk;
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    B_top b_mod{"b_mod"};
    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

