/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// break statement general cases
class top : sc_module
{
public:
    sc_in_clk clk;
    sc_signal<bool> arst;
    sc_signal<int> out{"out"};
    sc_signal<int> in{"in"};
    sc_signal<int> a;
    sc_signal<int> b;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(continue_in_for_wait0, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_in_for_wait1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_in_for_wait2, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_in_for_wait3, clk.pos());
        async_reset_signal_is(arst, 0);
        
        SC_CTHREAD(continue_break_exit1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_break_exit2, clk.pos());
        async_reset_signal_is(arst, 0);

        SC_CTHREAD(continue_in_while_wait1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_in_while_wait2, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_in_while_wait3, clk.pos());
        async_reset_signal_is(arst, 0);

        SC_CTHREAD(continue_in_while_for1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_in_while_for2, clk.pos());
        async_reset_signal_is(arst, 0);
        
        SC_CTHREAD(continue_in_do_while_wait1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(continue_in_do_while_wait2, clk.pos());
        async_reset_signal_is(arst, 0);
    }

    // Conditional @continue in for with wait()
    void continue_in_for_wait0()
    {
        int k = 0;
        wait();
        
        while (1) {

            for (int i = 0; i < 3 ; i++) {
                if (in.read() > 1) {
                    k = 1;
                    wait();  // 1
                    sct_assert_level(3);
                    continue;
                }
                sct_assert_level(2);
                
                k = 2;
                wait();  // 2
            }
            sct_assert_level(1);
            k = 3;
            wait(); // 3
        }
    }

    // Conditional @continue in for with wait()
    void continue_in_for_wait1()
    {
        int k = 0;
        wait();
        
        while (1) {

            for (int i = 0; i < 3 ; i++) { // B7
                k = 1;
                wait();  // 1

                if (in.read() > 1) {
                    k = 2;
                    continue;
                }
                k = 3;                  // B4
            }                           // B3
            sct_assert_level(1);
            wait();     // 2 
        }
    }

    // Conditional @continue in for with wait()
    void continue_in_for_wait2()
    {
        int k = 0;
        wait();
        
        while (1) {

            for (int i = 0; i < 3 ; i++) {
                if (in.read() > 1) {
                    wait();  // 1
                    k = 2;
                } else {
                    wait();  // 2
                    continue;
                }
                k = 1;
            }
            sct_assert_level(1);
            k = 3;
            wait(); // 3 
        }
    }

    // @continue in inner for loop
    void continue_in_for_wait3()
    {
        int k = 0;
        wait();
        
        while (1) {

            for (int i = 0; i < 3 ; i++) {
                for (int j = 0; j < 2 ; j++) {
                    k = 1;
                    wait();  // 1

                    if (in.read() > 1) {
                        k = 2;
                        sct_assert_level(4);
                        wait();   // 2
                    }

                    if (in.read() > 2) {
                        k = 3;
                        continue;
                    }
                    sct_assert_level(3);
                }    
            }
            sct_assert_level(1);
            k = 4;
            wait();   // 3
        }
    }
    
// ---------------------------------------------------------------------------

    // Continue in For loop with exit by break only, break in then branch
    void continue_break_exit1() {
        int k = 0;
        wait();
        
        while (1) {
            k = 0;
            for ( ; ; ) {
                wait();
                k = k + 1;
                if (a.read()) {
                    break;
                } else {
                    continue;
                }
                sct_assert_level(2);
            }
            sct_assert_level(1);
            k = 1;
        }
    }    

    // Continue in For loop with exit by break only, break in else branch
    void continue_break_exit2() {
        int k = 0;
        wait();
        
        while (1) {
            k = 0;
            for ( ; ; ) {
                if (a.read()) {
                } else {
                    break;
                }
                sct_assert_level(2);
                wait();     // 1
                
                k = k + 1;
                if (b.read()) continue;
            }
            sct_assert_level(1);
            wait();         // 2
            k = 1;
        }
    }  
    
// ---------------------------------------------------------------------------    
    
    void continue_in_while_wait1()
    {
        int k = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                if (in.read() > 1) {
                    k = 1;
                    wait();  // 1
                    continue;
                }
                
                i++;
                wait();  // 2
                sct_assert_level(2);
            }
            k = 3;
            wait(); // 3 
            sct_assert_level(1);
        }
    }    

    void continue_in_while_wait2()
    {
        int k = 0;
        wait();
        
        while (1) {
            int i = 0;
            while (i < 3) {
                k = 1;
                
                int j = 0;
                while (j < i) {
                    j++;
                    if (b.read()) continue;
                }
                wait();             // 1

                if (a.read()) {
                    k = 2;
                    if (b.read()) continue;
                }
                i++;
            }
            sct_assert_level(1);
            k = 3;
        }
    }   
    
    void continue_in_while_wait3()
    {
        int k = 0;
        wait();
        
        while (1) {

            while (a.read()) {
                k = 1;
                wait();     // 1

                if (a.read() && b.read()) {
                    k = 2;
                    continue;
                }
                k = 3;
            }
            sct_assert_level(1);
            k = 4;
            wait();         // 2
        }
    }
     
    // While and for loops with continue
    void continue_in_while_for1()
    {
        int k = 0;
        wait();
        
        while (1) {

            while (a.read() || b.read()) {
                if (b.read()) {
                    k = 1;
                    for (int i = 0; i < 3; i++) {
                        wait();     // 1
                        if (a.read()) continue;
                    }
                    k = 2;
                }
                wait();     // 2
                k = 3;
            }
            sct_assert_level(1);
            k = 4;
            wait();         // 3
        }
    }     
    
    
    // Break and continue chain
    void continue_in_while_for2()
    {
        int k = 0;
        wait();
        
        while (1) {

            while (a.read() || b.read()) {
                k = 1;
                for (int i = 0; i < 3; i++) {
                    if (a.read()) break;
                    wait();     // 1
                }
                wait();         // 2
                
                if (a.read()) continue;
                k = 3;
            }
            sct_assert_level(1);
            k = 4;
            wait();             // 3
        }
    } 

// ---------------------------------------------------------------------------    
    
    // do/while with @continue 
    void continue_in_do_while_wait1()
    {
        int k = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                wait();  // 1

                if (a.read()) continue;
                k = 2;
            } while (i < 3);
            sct_assert_level(1);

            k = 1;
            wait();     // 2
        }
    }    
    
    
    void continue_in_do_while_wait2()
    {
        int k = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                if (b.read()) {
                    wait(); // 1
                    continue;
                }
                k = 2;
                wait(); // 2
                
            } while (i < 3 || a.read());
            sct_assert_level(1);

            k = 1;
            wait();         // 2
        }
    }   
    
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(200, SC_NS);
    return 0;
}

