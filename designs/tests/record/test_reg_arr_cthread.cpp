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
    int sin;
    int cos;
    sc_biguint<65> m;
    
    inline bool type() const {
        return m.bit(42);
    }
    
    inline bool isBwdUser() const {
        return type();
    }
};

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    sc_signal<int> s;
    
    SC_CTOR(A)
    {
        SC_CTHREAD(record_arr_comb, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_arr_reg1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_arr_reg2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_arr_glob_reg, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    // Local record array non-registers
    sc_signal<int> t2;
    void record_arr_comb() {
        t2 = 0;
        wait();
        
        while (true) {
            int uu[2];
            SinCosTuple l;          // No register should be generated 
            SinCosTuple ll[2];      // for @l and @ll

            t2 = uu[s.read()];
            t2 = l.sin;
            t2 = ll[s.read()].sin;
            t2 = ll[s.read()].m.to_int();
            if (ll[s.read()].isBwdUser()) { t2 = 1; }
            
            wait();
        }
    }

    // Local record array before wait() becomes registers
    sc_signal<int> t3;
    void record_arr_reg1() {
        t3 = 0;
        wait();
        
        while (true) {
            SinCosTuple nn[2];      
            wait();
            t3 = nn[s.read()].sin;
        }
    }
    
    // Local record array registers
    void record_arr_reg2() {
        
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
    sc_signal<int> t0;
    SinCosTuple gra[2];
    void record_arr_glob_reg() {
        wait();
        
        while (true) {
            gra[0].sin = 1;
            wait();
            int b = gra[0].sin;
            t0 = b;
        }
    }
    
    sc_signal<int> t1;
    void arr_reg() {
        
        int r[2];
        r[0] = 1; r[1] = 2;
        wait();
        
        while (true) {
            int i = s.read();
            int b = r[i];
            t1 = b;
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

