/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// do..whjle infinite and break/continue in do..while
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> out{"out"};
    sc_signal<int> in{"in"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(dowhile_forever);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(for_ever);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_break1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_break2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(dowhile_break3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_continue1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(dowhile_continue2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    // @do..while with wait
    void dowhile_forever()
    {
        out = 0;
        wait();
        
        do {             // B7

            int i = 0;          // B6
            do {
                out = 1;        // B4    
                wait();  // 2
                i++;
            } while (i < 3);    // B3, B5
            
            out = 2;            // B2, B1
        } while(1);
    }
    

    // @while with inner @for 
    sc_signal<int> s1;
    void for_ever()
    {
        s1 = 0;
        wait();
        
        for (;;) {

            int i = 0;
            do {
                i++;
                s1 = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        s1 = j;
                    }
                    wait();  // 2
                }
            } while (i < 3);
            s1 = 3;
            wait();     // 3
        }
    }
    
    sc_signal<int> s2;
    void dowhile_break1()
    {
        s2 = 0;
        int i = 0;
        wait();
        
        while (true) {

            do {
                if (i > 3) {
                    wait();     // 2
                    break;
                }
                
                i++;
                s2 = i;
                wait();         // 1
                
            } while (true);
            
            wait();             // 3
        }
    }
    
    sc_signal<int> s3;
    void dowhile_break2()
    {
        s3 = 0;
        wait();
        
        while (true) {

            do {
                s3 = s2.read();
                wait();
                
                if (s2.read()) {
                    break;
                } else {
                    continue;
                }
                wait();         
                
            } while (true);
            
            wait();          
        }
    }
    
    sc_signal<int> s4;
    void dowhile_break3()
    {
        s4 = 0;
        wait();
        
        while (true) {

            do {
                s4 = s1;
                wait();             // 1
                
                if (s1.read()) {
                    break;
                }
                wait();             // 2
                
            } while (true);
            
            do {
                if (s2.read()) {
                    s4 = 1;
                    break;
                }
                s4 = s2;
                wait();             // 3
                
            } while (true);
            
            wait();                 // 4
        }
    }    
    
    
    sc_signal<int> s5;
    void dowhile_continue1()
    {
        s5 = 0;
        int i = 42;
        wait();
        
        while (true) {

            do {
                do {
                    i--;
                    wait();         // 1
                    if (i > s3) continue;
                    
                } while (s4.read());
                
                i = s1.read();
                
                if (s2.read()) {
                    s5 = 1;
                    continue;
                }
                wait();             // 2
                
            } while (false);
            
            wait();                 // 3
        }
    }    
    
    sc_signal<int> s6;
    void dowhile_continue2()
    {
        s6 = 0;
        int i = 42;
        wait();
        
        while (true) {

            do {
                do {
                    i--;
                    wait();         // 1
                    if (i > s3) continue;
                } while (false);
                
                i = s1.read();
                
                if (s2.read()) {
                    s6 = 1;
                    continue;
                }
                wait();             // 2
                
            } while (s2.read());
            
            wait();                 // 3
        }
    }    
    

};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

