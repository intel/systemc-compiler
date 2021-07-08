/******************************************************************************
* Copyright (c) 2021, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// NO reset CTHREAD with reset code, multiple states -- error reported 
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> nrst;
    
    sc_signal<bool> s;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(control_proc, clk.pos());
        
        SC_CTHREAD(reset_wait1, clk.pos());
        SC_CTHREAD(reset_wait2, clk.pos());

        SC_CTHREAD(reset_code0, clk.pos());
        SC_CTHREAD(reset_code1, clk.pos());
        SC_CTHREAD(reset_code2, clk.pos());
        SC_CTHREAD(reset_code3, clk.pos());
        SC_CTHREAD(reset_code4, clk.pos());
        
        SC_CTHREAD(multi_state1, clk.pos());
        SC_CTHREAD(multi_state2, clk.pos());
        SC_CTHREAD(multi_state3, clk.pos());
        SC_CTHREAD(multi_state4, clk.pos());
    }
    
    void control_proc() 
    {
        while (true) {
            int k = 0;
            wait();
        }
    }
    
    // Reset wait before main loop
    void reset_wait1() 
    {
        int l = 0; 
        wait();
        while (true) {
            l++;
            wait();
        }
    }
    
    void reset_wait2() 
    {
        s = 0; 
        wait();
        while (true) {
            s = !s;
            wait();
        }
    }

    // Code before main loop
    void reset_code0() 
    {
        if (s.read()) {} 
        while (true) {
            int k = 0;
            wait();
        }
    }
    
    void reset_code1() 
    {
        int l = 0; 
        while (true) {
            l++;
            wait();
        }
    }
    
    sc_signal<bool> t;
    void reset_code2() 
    {
        t = 1;
        
        while (true) {
            t = 0;
            
            if (!s) {
                t = 1;
            }
            
            wait();
        }
    }
    
    // Common wait() not as the last statement
    void reset_code3() 
    {
        while (true) {
            int ll = 1;
            wait();
            if (s.read()) ll++;
        }
    }
    
    
    sc_signal<bool> v;
    void reset_code4() 
    {
        while (true) {
            v = 0;
            
            wait();

            v = t.read() ? s.read() : 0;
        }
    }
    
    // Multiple states
    void multi_state1() 
    {
        while (true) {
            sc_uint<3> i = 0;
            wait();
            
            i += s.read();
            wait();
        }
    }
    
    void multi_state2() 
    {
        while (true) {
            while (s.read()) wait();
            
            int i = s.read();
            wait();
        }
    }
    
    void multi_state3() 
    {
        while (true) {
            wait();
            int i = s.read();
            
            if (i) {
                int j = t.read();
                wait();
            }
        }
    }
    
    // wait(N)
    void multi_state4() 
    {
        while (true) {
            int i = s.read();
            wait(3);
        }
    }
    
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

