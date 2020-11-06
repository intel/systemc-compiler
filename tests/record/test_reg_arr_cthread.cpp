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
        SC_CTHREAD(record_arr_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_arr_glob_reg, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    // Local record array registers
    void record_arr_reg() {
        
        SinCosTuple r[2];
        wait();
        
        while (true) {
            int i = s.read();
            int b = r[i].sin;
            wait();
            r[1].sin = 3;
        }
    }
    

    // Global record array register
    SinCosTuple gra[2];
    void record_arr_glob_reg() {
        wait();
        
        while (true) {
            gra[0].sin = 1;
            wait();
            int b = gra[0].sin;
        }
    }
    
     void arr_reg() {
        
        int r[2];
        r[0] = 1; r[1] = 2;
        wait();
        
        while (true) {
            int i = s.read();
            int b = r[i];
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

