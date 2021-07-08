/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// do/while general cases
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
        SC_THREAD(dowhile_with_wait0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(dowhile_with_wait0a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(dowhile_with_wait1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(dowhile_with_wait2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_with_for);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_with_signal_cond);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_inner1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_inner2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(dowhile_inner3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(complex1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    // @do..while with wait
    void dowhile_with_wait0()
    {
        out = 0;
        wait();
        
        while (1) {             // B7

            int i = 0;          // B6
            do {
                out = 1;        // B4    
                wait();  // 1
                i++;
            } while (i < 3);    // B3, B5
            
            out = 2;            // B2, B1
        }
    }
    
    sc_signal<int> s0;
    void dowhile_with_wait0a()
    {
        s0 = 0;
        wait();
        
        while (1) {             
            int i = 0;          
            do {
                s0 = i;
                i++;
                wait();         
            } while (i < 3);    
        }
    }
    
    // @while with waits
    sc_signal<int> s1;
    void dowhile_with_wait1()
    {
        s1 = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                s1 = 1;
                wait();     // 1
            } while (i < 3);
            s1 = 2;
            wait();         // 2
        }
    }
    
    // @while with conditional wait
    sc_signal<int> s2;
    void dowhile_with_wait2()
    {
        s2 = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                s2 = 1;
                wait();     // 2
                
                if (in.read() > 1) {
                    s2 = 2;
                    wait();  // 3
                }
            } while (i < 3);
            s2 = 3;
            wait();     // 4
        }
    }
    
    // @while with inner @for 
    sc_signal<int> s3;
    void dowhile_with_for()
    {
        s3 = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                s3 = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        s3 = j;
                    }
                    wait();  // 1
                }
            } while (i < 3);
            s3 = 3;
            wait();         // 2
        }
    }

    // @while with signal condition
    sc_signal<int> s4;
    void dowhile_with_signal_cond()
    {
        s4 = 0;
        wait();
        
        while (1) {

            do {
                s4 = 1;
                wait();     // 2
            } while (in.read());

            s4 = 2;
            wait();     // 3
        }
    }
    
// ----------------------------------------------------------------------------    
    
    // Inner do...while
    sc_signal<int> s6;
    void dowhile_inner1()
    {
        s6 = 0;
        wait();
        
        while (1) {

            do {
                s6 = 1;
                
                int i = 3;
                do {
                    i++;
                    s6 = i-1;
                    wait();         // 1
                    
                } while (i < 10);
                
            } while (in.read());
            
            wait();                 // 2
        }
    }
    
    sc_signal<int> s7;
    void dowhile_inner2()
    {
        s7 = 0;
        wait();
        
        while (1) {

            do {
                s7 = 1;
                wait();             // 1
                
                int i = 3;
                do {
                    i++;
                    wait();         // 2

                    s7 = i-1;
                } while (i < 10);
                
            } while (s7.read() > 5);
            
            s7 = 0;
            wait();                 // 3
        }
    }    
    
    
    sc_signal<int> s8;
    void dowhile_inner3()
    {
        s8 = 42;
        wait();
        
        while (1) {

            int k = 0;
            do {
                int i = 0;
                if (in.read()) {
                    do {
                        i++;
                        s8 = i;
                        wait();     // 1
                    } while (s8.read() < 10);
                    
                } else {
                    k++;
                    wait();         // 2
                }
                
            } while (k < 10);
            
            wait();                 // 3
        }
    }
    
// ----------------------------------------------------------------------------    
    
    sc_signal<int> s5;
    void complex1()
    {
        s5 = 0;
        wait();
        while (1) {

            int i = 0;
            do {
                i++;
                if (i > 3)
                    break;
                if (i > 4)
                    continue;

                i++;
            } while (i < 1);

            do {
                i ++;
                s5 = i;
            } while ( i < 5);

            i = 0;

            do {
                i ++;
                s5 = i;

                if (in.read())
                    break;

                wait();         // 1

            } while (i < 3);

            do {
                wait();         // 2
            } while (in.read());

            cout << "tick\n";
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

