/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Local records in thread with field used as registers
struct SinCosTuple 
{
    int sin = 1;
    int cos = 2;
};

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    sc_signal<int> s;
    
    SC_CTOR(A)
    {
        SC_CTHREAD(record_assign, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_usedef_assign, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_glob_reg, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(record_glob_assign1, clk.pos());
        async_reset_signal_is(rstn, false);

        // #141
        //SC_CTHREAD(record_glob_assign1a, clk.pos());
        //async_reset_signal_is(rstn, false);

        // #141
        //SC_CTHREAD(record_glob_assign2, clk.pos());
        //async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(record_not_defined, clk.pos());
        async_reset_signal_is(rstn, false);
        // TODO: Fix me, see #211
        SC_CTHREAD(record_glob_not_defined, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    // Local records assign
    void record_assign() {
        SinCosTuple s;
        wait();
        
        while (true) {
            s.sin = 1;
            SinCosTuple r = s;
            wait();
            
            int b = r.sin;
        }
    }
    
    void record_usedef_assign() {
        wait();
        
        while (true) {
            SinCosTuple r;
            wait();

            int i = r.sin;
        }
    }

    // Global record register
    SinCosTuple gr;
    void record_glob_reg() {
        wait();
        
        while (true) {
            gr.sin = 1;
            wait();
            int b = gr.sin;
        }
    }
    
    // Local/global records assign in initialization
    SinCosTuple grr;
    void record_glob_assign1() {
        wait();
        
        while (true) {
            grr.cos = 1;
            SinCosTuple r = grr;
            wait();
            
        }
    }
    
    // Local/global records assign
    SinCosTuple gp;
    void record_glob_assign1a() {
        wait();
        
        while (true) {
            SinCosTuple r;
            r = gp;
            wait();
            
        }
    }
    
    SinCosTuple gpp;
    void record_glob_assign2() {
        wait();
        
        while (true) {
            SinCosTuple r;
            wait();
            
            gpp = r;
        }
    }
    
    // Local record not defined
    void record_not_defined() {
        wait();
        
        while (true) {
            SinCosTuple rn;
            wait();
            int i = rn.sin;     // @rn must be register
        }
    }

    // Global record not defined, no fields initialized see #211
    SinCosTuple gn;
    void record_glob_not_defined() {
        wait();
        
        while (true) {
            SinCosTuple r = gn;
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

