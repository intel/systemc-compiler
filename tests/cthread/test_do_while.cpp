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
                wait();  // 2
                i++;
            } while (i < 3);    // B3, B5
            
            out = 2;            // B2, B1
        }
    }
    
      // @while with wait
    void dowhile_with_wait1()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                out = 1;
                wait();     // 1
            } while (i < 3);
            out = 2;
            wait();         // 2
        }
    }
    
    // @while with conditional wait
    void dowhile_with_wait2()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                out = 1;
                wait();     // 2
                
                if (in.read() > 1) {
                    out = 2;
                    wait();  // 3
                }
            } while (i < 3);
            out = 3;
            wait();     // 4
        }
    }
    
    // @while with inner @for 
    void dowhile_with_for()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                out = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        out = j;
                    }
                    wait();  // 2
                }
            } while (i < 3);
            out = 3;
            wait();     // 3
        }
    }

    // @while with signal condition
    void dowhile_with_signal_cond()
    {
        out = 0;
        wait();
        
        while (1) {

            do {
                out = 1;
                wait();     // 2
            } while (in.read());

            out = 2;
            wait();     // 3
        }
    }
    
    void complex1()
    {
        out = 0;
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
                out = i;
            } while ( i < 5);

            i = 0;

            do {
                i ++;
                out = i;

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

