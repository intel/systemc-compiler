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
    sc_in<bool> clk;
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};
    sc_signal<int> s;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(while_with_wait0, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_wait0a, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_wait0b, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_wait1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_wait2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_for, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_signal_cond, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(while_with_binary_oper, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_binary_oper1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_binary_oper2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_binary_oper3, clk.pos());
        async_reset_signal_is(arstn, false);
        
        // TODO: Fix me, #133 -- error reported
        //SC_CTHREAD(no_wait_main_loop, clk.pos());
        //async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(while_continue1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_break1, clk.pos());
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
    sc_signal<int> tt1;
    void while_with_wait2()
    {
        tt1 = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                tt1 = 1;
                wait();     // 1
                
                if (in.read() > 1) {
                    tt1 = 2;
                    wait();  // 2
                }
            }
            tt1 = 3;
            wait();     // 3
        }
    }
    
    // @while with inner @for 
    sc_signal<int> tt2;
    void while_with_for()
    {
        tt2 = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                tt2 = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        tt2 = j;
                    }
                    wait();  // 1
                }
            }
            tt2 = 3;
            wait();     // 2
        }
    }

    // @while with signal condition
    sc_signal<int> tt3;
    void while_with_signal_cond()
    {
        tt3 = 0;
        wait();
        
        while (1) {

            while (in.read()) {
                tt3 = 1;
                wait();     // 0
            }

            tt3 = 2;
            wait();         // 0
        }
    }

    // While with binary ||/&& operator -- BUG in real design EMC
    sc_signal<int> t0;
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
            t0 = k;
            wait();             // B2, B1
        }
    }
    
    sc_signal<int> t1;
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
            t1 = k;
            wait();             
        }
    }
    
    // While with binary ||/&& operator -- BUG in real design EMC fixed
    sc_signal<int> t2;
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
            t2 = k;
            wait();         // B2
        }
    }
    
    sc_signal<int> t3;
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
            t3 = k;
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
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

