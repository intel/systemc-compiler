/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_static_log.h"
#include <systemc.h>

const unsigned EifWrBLenMax    = 16;
const unsigned EifWrBLenMaxL2  = sct::sct_addrbits<EifWrBLenMax>;
const unsigned TileDimWidth    = 12;
const unsigned TileDimsNum     = 4;
const unsigned EdmaStoreSlots  = 2;
const unsigned DmeOperWidth    = 2;
const unsigned EdmaStSltPtrWidth = sct::sct_addrbits1<EdmaStoreSlots>;

typedef sc_uint<TileDimsNum * TileDimWidth>          TileDims_t;
typedef sc_uint<EdmaStSltPtrWidth> EdmaStSltIdx_t;
typedef sc_uint<TileDimWidth>    TileDim_t;
    

// Function call in constant initialization (bug fixed)
struct A : public sc_module 
{
    sc_in<bool> clk;
    sc_signal<bool> nrst;
    
    sc_signal<bool>           q;
    sc_signal<sc_uint<3>>     s;
    sc_signal<TileDims_t>     t;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name, bool b_, int i_) : 
        sc_module(name) 
    {
        SC_CTHREAD(stExtIfPushFsmProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    TileDim_t getDim(TileDims_t dims, unsigned n)
    {
        return dims.range((n + 1) * TileDimWidth - 1, n * TileDimWidth);
    }
    
    void stExtIfPushFsmProc()
    {
        const bool burst = q;                           // B11
        const TileDim_t N0 = burst ?                    // B8
                             TileDim_t(getDim(t, 0)) :  // B9
                             TileDim_t(0);              // B10

        wait();                     // B8

        while (true) {              // B7
            TileDim_t x0 = 0;       // B6
            do {                    // B5
                x0 += 1;            // B4

                // 1. Duplicate loop under bug
                //wait();

                // 2. Hangs up  under bug
                do {
                    wait();
                } while (!q);

            } while (x0 <= N0);     // B3

            wait();                 // B2
        }                           // B1
    }                               // B0
};


int sc_main(int argc, char **argv) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A modA{"modA", 1, 11};
    modA.clk(clk);
    
    sc_start();

    return 0;
}

