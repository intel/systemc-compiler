/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Test for bit/range access in LHS
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    sc_uint<2>  arr[2];
    sc_uint<3>  arr2[3][4];
    
    sc_uint<3>  a;
    sc_uint<4>  b;
    sc_uint<5>  c;
    sc_uint<6>  d;
    
    const sc_uint<7>  e = 42;
    sc_uint<8>  f;
    
    sc_uint<9>* p;
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        p = sc_new<sc_uint<9> >();
                
        SC_THREAD(comb_var_not_changed);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
          
        SC_THREAD(comb_var_not_changed1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false); 

        SC_THREAD(comb_var_in_reset0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(comb_var_in_reset0a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(comb_var_in_reset1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_var_in_reset1a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(comb_var_in_reset1b);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(comb_var_in_reset2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_var_in_reset2a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_var_in_reset3);
        sensitive << clk.posedge_event();
        // No reset signal
        
        SC_THREAD(comb_var_in_reset3a);
        sensitive << clk.posedge_event();
        // No reset signal

        SC_THREAD(loc_comb_var_in_reset);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(readonly_var_in_reset);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);        

        SC_THREAD(readonly_var_in_reset2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);       
    }

    // BUG in real design -- fixed
    sc_uint<3> popIndx;
    void comb_var_not_changed()
    {
        popIndx = 0;
        wait();
        
        while (true) {
            out = arr[popIndx];
            wait();
        }
    }
    
    void comb_var_not_changed1()
    {
        popIndx = 0;
        
        while (true) {
            wait();
            out = arr[popIndx];
        }
    }
    

    // Combinational variable with initialization in reset
    void comb_var_in_reset0()
    {
        a = 0;
        wait();
        
        while (true) {
            a = 3;
            wait();
        }
    }

    // Combinational variable with initialization in reset
    // Multiple states
    void comb_var_in_reset0a()
    {
        a = 0;
        wait();
        
        while (true) {
            a = 3;
            wait();

            a = 4;
            wait();
        }
    }

    // Combinational variable with initialization in reset
    void comb_var_in_reset1()
    {
        b = 0;

        while (true) {
            wait();
            b = 3;
        }
    }
    
    // Combinational variable with initialization in reset
    // Multiple states
    void comb_var_in_reset1a()
    {
        b = 0;

        while (true) {
            wait();
            b = 3;

            wait();
            b = 4;
        }
    }

    // Combinational variable with initialization in reset
    // Multiple states, variable assigned only at some state
    void comb_var_in_reset1b()
    {
        b = 0;

        while (true) {
            wait();
            b = 3;

            wait();
        }
    }
    
    // Combinational variable with initialization in reset -- no reset section
    void comb_var_in_reset2()
    {
        while (true) { 
            c = 3;
            wait();
        }
    }

    // Combinational variable with initialization in reset -- no reset section
    // Multiple states
    void comb_var_in_reset2a()
    {
        while (true) { 
            c = 3;
            wait();
            
            c = 4;
            wait();
        }
    }
    
    // Combinational variable with initialization in reset -- no reset signal
    void comb_var_in_reset3()
    {
        while (true) { 
            d = 3;
            wait();
        }
    }
    
    // Combinational variable with initialization in reset -- no reset signal
    // Multiple states
    void comb_var_in_reset3a()
    {
        while (true) { 
            d = 3;
            wait();

            d = 4;
            wait();
        }
    }

    void loc_comb_var_in_reset()
    {
        int lc = 0;
        wait();
        
        while (true) {
            lc = 3;
            wait();
        }
    }
    
    // Read-only constant with initialization in reset
    void readonly_var_in_reset()
    {
        int lc = e;
        wait();
        
        while (true) {
            lc = e+1;
            wait();
        }
    }

    // Read-only variable with initialization in reset
    void readonly_var_in_reset2()
    {
        f = 43;
        int lc = f;
        wait();
        
        while (true) {
            lc = f+1;
            wait();
        }
    }
 };

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    
    sc_start(100, SC_NS);
    return 0;
}

