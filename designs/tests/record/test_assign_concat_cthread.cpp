/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Record (structure/class) assignment and field concatenation in CTHREAD
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    sc_signal<int> sig;

    SC_CTOR(A) 
    {
        SC_CTHREAD(record_assign1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_assign2, clk.pos());
        async_reset_signal_is(rstn, false);
        
        // #141
        //SC_CTHREAD(record_assign3, clk.pos());   
        //async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(record_concat_reg, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(record_concat_comb, clk.pos());
        async_reset_signal_is(rstn, false);
        
    }
    
    struct Simple {
        bool a;
        int b;
    };
    
    // Record assignment in declaration and binary operator
    void record_assign1() 
    {
        Simple s;               // reg
        s.a = true; s.b = 42;
        wait();
        
        while (true) {
            Simple r = s;
            r = s;
            
            wait();
        }
    }
    
    // Record assignment in declaration and binary operator
    void record_assign2() 
    {
        wait();
        
        while (true) {
            Simple s;           // comb
            Simple r = s;
            s = r;
            
            wait();
        }
    }

    // Record assignment in declaration and binary operator with member record
    Simple t;
    void record_assign3() 
    {
        t.a = true; t.b = 42;
        Simple v = t;
        wait();
        
        while (true) {
            Simple w = t;
            t = v;          // #141, @v is not declared, add it to Use
            
            wait();
        }
    }

    void record_concat_reg() 
    {
        Simple x;
        x.b = 42;
        wait();
        
        while (true) {
            Simple y;
            y.b = ((sc_uint<1>)x.a, (sc_uint<2>)x.b);
            wait();
        }
    }
        
    void record_concat_comb() 
    {
        wait();
        
        while (true) {
            Simple z;
            Simple f;
            f.b = 42;
            z.b = ((sc_uint<1>)f.a, (sc_uint<2>)f.b);
            wait();
        }
    }
    
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);

    sc_start();
    return 0;
}

