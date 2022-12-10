/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Port/signal pointers, arrays and pointer arrays as function parameter
struct Top : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    
    sc_signal<bool>     s;

    SC_HAS_PROCESS(Top);
    Top (const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(methProc1); sensitive << s;
        SC_METHOD(methProc2); sensitive << s;
        SC_METHOD(methProc3); sensitive << s;
        SC_METHOD(methProc4); sensitive << s;
        SC_METHOD(methProc5); sensitive << s;
        SC_METHOD(methProc6); sensitive << s;
        SC_METHOD(methProc7); sensitive << s;
        SC_METHOD(methProc8); sensitive << s;

        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rst, 0);
    }
    
    static const unsigned TileDimWidth    = 12;
    static const unsigned TileDimsNum     = 4;
    static const unsigned EdmaStoreSlots = 2;
    typedef sc_uint<TileDimWidth>    TileDim_t;
    typedef sc_uint<TileDimsNum * TileDimWidth>  TileDims_t;
    static const unsigned UcAddrWidth = 20; 
    typedef sc_uint<UcAddrWidth> UcAddr_t;
    
    sc_vector<sc_signal<TileDims_t>>   st_slt_dims{"st_slt_dims", EdmaStoreSlots};
    
    void f(int i)
    {
        int l = i;
    }
    
    void methProc1() 
    {
        for (int i = 0; i < 2; ++i) {
            f(i);
        }
        f(0);
    }

    void methProc2() 
    {
        f(0);
        for (int i = 0; i < 2; ++i) {
            f(i);
        }
    }
    
    void methProc3() 
    {
        for (int i = 0; i < 2; ++i) {
            f(i);
        }
        for (int i = 0; i < 2; ++i) {
            f(i+1);
        }
    }

    void methProc4() 
    {
        f(0);
        for (int i = 0; i < 2; ++i) {
            f(i);
        }
        for (int i = 0; i < 2; ++i) {
            f(i+1);
        }
    }
    
    void methProc5() 
    {
        for (int i = 0; i < 2; ++i) {
            f(i);
        }
        f(0);
        for (int i = 0; i < 2; ++i) {
            f(i+1);
        }
    }
    
    void methProc6() 
    {
        for (int i = 0; i < 2; ++i) {
            f(i);
            for (int i = 0; i < 3; ++i) {
                f(i+1);
            }
        }
    }
    
    void methProc7() 
    {
        for (int i = 0; i < 2; ++i) {
            for (int i = 0; i < 3; ++i) {
                f(i+1);
            }
            f(i);
        }
    }
    
    // This example shows every loop must have unique counter name
    void methProc8() 
    {
        for (int i = 0; i < 2; ++i) {
            int &m = i;
            for (int i = 0; i < 3; ++i) {
                int k = i + m; 
            }
        }
    }
    
// --------------------------------------------------------------------------
    
    TileDim_t getDim(TileDims_t dims, unsigned i)
    {
        return dims.range((i + 1) * TileDimWidth - 1, i * TileDimWidth);
    }    
    
    bool incElIdx(TileDims_t &xes, TileDims_t dims)
    {
        TileDim_t  xes_nxt[TileDimsNum];
        for (unsigned i = 0; i < TileDimsNum; ++i) {
            xes_nxt[i]  = getDim(xes, i);
        }
        return xes_nxt[0];
    }
    
    void threadProc() 
    {
        unsigned slot = 0;
        wait();
        
        while (true) 
        {
            TileDims_t xes = 0;
            bool xtc = false;

            do {
                UcAddr_t ax0 = getDim(xes, 0);
                
                xtc = incElIdx(xes, st_slt_dims[slot]);
                wait();
            } while (!xtc);
            
            slot = (slot == EdmaStoreSlots - 1) ? 0 : slot + 1;
            wait();
        } 
    }
    
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    sc_signal<bool> rst{"rst"};
    
    Top top{"top"};
    top.clk(clock_gen);
    top.rst(rst);
    sc_start();

    return 0;
}

