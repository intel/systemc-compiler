/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// while general cases and binary condition
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};
    sc_signal<int> s;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(while_with_wait0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_wait0a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
          
        SC_THREAD(while_with_wait0b);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_wait1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_wait2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_for);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_signal_cond);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(while_with_binary_oper);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_binary_oper1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_binary_oper2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_binary_oper3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        // TODO: Fix me, #133 -- error reported
        //SC_THREAD(no_wait_main_loop);
        //sensitive << clk.posedge_event();
        //async_reset_signal_is(arstn, false);
        
        SC_THREAD(while_continue1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_break1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    // @while with wait
    void while_with_wait0()
    {
        wait();
        
        while (1) { // B6

            int i = 0;  // B5
            while (i < 3) { // B4
                wait();     // B3
                i++;
            }           // B2
        }   // B1
    }
    
    // @while with wait and global iteration counter
    void while_with_wait0a()
    {
        int i = 0;  
        wait();
        
        while (1) { 
            while (i < 3) {
                wait();     
                i++;
            }
            wait();
        }   
    }
    
    void while_with_wait0b()
    {
        int i = 0;  
        wait();
        
        while (1) { 
            while (i < s.read()) {
                wait();     
                i++;
            }
            wait();
        }   
    }
    
    // @while with wait
    void while_with_wait1()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                out = 1;
                wait();     // 2
            }
            out = 2;
            wait();     // 3
        }
    }
    
    // @while with conditional wait
    void while_with_wait2()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                out = 1;
                wait();     // 1
                
                if (in.read() > 1) {
                    out = 2;
                    wait();  // 2
                }
            }
            out = 3;
            wait();     // 3
        }
    }
    
    // @while with inner @for 
    void while_with_for()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                out = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        out = j;
                    }
                    wait();  // 1
                }
            }
            out = 3;
            wait();     // 2
        }
    }

    // @while with signal condition
    void while_with_signal_cond()
    {
        out = 0;
        wait();
        
        while (1) {

            while (in.read()) {
                out = 1;
                wait();     // 1
            }

            out = 2;
            wait();     // 2
        }
    }

    // While with binary ||/&& operator -- BUG in real design EMC
    void while_with_binary_oper()
    {
        bool b1, b2;
        int k = 0;
        wait();
        
        while (1) {             // B7
            while (b1 || b2) {  // B6, B5
                k = 1;        // B4
                wait();
                k = 2;
            }                   // B3
            wait();             // B2, B1
        }
    }
    
    void while_with_binary_oper1()
    {
        bool b1, b2;
        int k = 0;
        wait();
        
        while (1) {             
            while (b1 && s.read()) {  
                k = 1;          
                wait();
                k = 2;
            }                   
            wait();             
        }
    }
    
    // While with binary ||/&& operator -- BUG in real design EMC fixed
    void while_with_binary_oper2()
    { 
        bool b1, b2, b3;
        int k = 0;
        wait();     // B9
        
        while (1) {         // B8
            while ((b1 || b2) && b3) {  // B7, B6, B5
                k = 1;
                wait();     // B4
                k = 2;
            }               // B3
            wait();         // B2
        }
    }
    
    void while_with_binary_oper3()
    { 
        bool b1, b2, b3;
        int k = 0;
        wait();     
        
        while (1) { 
            while ((b1 && s.read()) || b3) { 
                k = 1;
                wait();     
                k = 2;
            }               
            wait();         
        }
    }
            
    
    // Main loop w/o wait(), see #133
    void no_wait_main_loop()
    {
        s = 0;
        wait();
        
        while (true) { 
            s = 1;
        } 
    }
    
// --------------------------------------------------------------------------
    
    // break and continue
    
    sc_signal<int> s1;
    sc_signal<int> s2;
    void while_continue1()
    { 
        s2 = 0;
        wait();

        while (true) 
        {
            while (!s.read()) {
                wait();             // 1
                s2 = 2;
            }

            while (s.read()) {
                wait();            // 2     
                if (s1.read()) {
                    s2 = 1;
                    continue;
                }
            }
            wait();                // 3       
        }
    }
    
    sc_signal<int> s3;
    void while_break1()
    { 
        wait();

        while (true) 
        {
            s3 = 0;
            while (true) {
                wait();             // 1     
                if (s1.read()) {
                    break;
                }
                s3 = 1;
            }
            while (s1) {
                while (s2) {
                    wait();         // 2
                }
                if (s1 > s2) { 
                    wait();         // 3
                    break;
                }
                s3 = 2;
                wait();             // 4
            }
            
            wait();                 // 5
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

