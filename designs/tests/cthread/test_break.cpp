/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// break statement in loops general cases
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
        SC_CTHREAD(code_after_break, clk.pos());
        async_reset_signal_is(arst, 0);

        SC_METHOD(break_level_meth);
        sensitive << a;
        
        SC_CTHREAD(break_level, clk.pos());
        async_reset_signal_is(arst, 0);
        
        SC_CTHREAD(break_in_for_wait0, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_for_wait1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_for_wait2, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_for_wait3, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_for_wait4, clk.pos());
        async_reset_signal_is(arst, 0);
        
        SC_CTHREAD(break_exit1, clk.pos());
        async_reset_signal_is(arst, 0);

        SC_CTHREAD(break_exit2, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_exit2a, clk.pos());
        async_reset_signal_is(arst, 0);

        SC_CTHREAD(break_exit3, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_exit4, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_exit4a, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_exit5, clk.pos());
        async_reset_signal_is(arst, 0);
        
        SC_CTHREAD(break_in_while_wait1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_while_wait2, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_while_wait3, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_while_wait4, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_while_for, clk.pos());
        async_reset_signal_is(arst, 0);
        
        SC_CTHREAD(break_in_do_while_wait1, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_do_while_wait2, clk.pos());
        async_reset_signal_is(arst, 0);
        SC_CTHREAD(break_in_do_while_wait3, clk.pos());
        async_reset_signal_is(arst, 0);
    }
    
    // Code after break ignored
    void code_after_break()
    {
        int k = 0;
        wait();
        
        while (1) {

            while (a.read() || b.read()) {
                if (b.read()) {
                    k = 1;
                    break;
                    k = 2;  // Code after break leads to error
                }
                wait();     // 1
                k = 3;
            }
            k = 4;
            wait();         // 2
        }
    }

    void break_level_meth() 
    {
        int k = 0;
        for (int i = 0; i < 4; i++) {  
            
            if (a.read()) {         // B6
            } else {                // B5
                break;              // B4
            }
            
            k = 1;                  // B3
            sct_assert_level(1);
        }                           // B2
        k = 2;                      // B1
    }    
    
    void break_level() {
        int k = 0;
        wait();
        
        while (1) {
            for (int i = 0; i < 4; i++) {               
                if (a.read()) {         // B7
                } else {                // B6
                    break;              // B5
                }
                k = 1;                  // B4
                sct_assert_level(2);
            }                           // B3
            k = 2;                      // B2
            wait();           
        }
    }    
        
// ---------------------------------------------------------------------------    

    // Conditional @break in for with wait()
    void break_in_for_wait0()
    {
        int k = 0;
        wait();
        
        while (1) {

            for (int i = 0; i < 3 ; i++) {
                if (in.read() > 1)
                    break;

                sct_assert_level(2);
                k = 2;
                wait();  // 1
            }
            sct_assert_level(1);
            k = 1;
            wait(); // 2 
        }
    }
    
    // Conditional @break after wait
    void break_in_for_wait1()
    {
        int k = 0;
        wait();
        
        while (1) {

            for (int i = 0; i < 3 ; i++) {  // B6
                k = 2;  // B5
                wait();                 // 1

                sct_assert_level(2);
                if (in.read() > 1) {
                    sct_assert_level(3);
                    break;      // B4
                }
            }       // B3
            k = 1;  // B2
            sct_assert_level(1);
            wait();                     // 2 
        }
    }
    
    
    // Conditional @break in for with wait()
    void break_in_for_wait2()
    {
        int k = 0;
        wait();
        
        while (1) {

            for (int i = 0; i < 3 ; i++) {
                if (in.read() > 1) {
                    k = 1;
                    sct_assert_level(3);
                    wait();  // 1
                    break;
                }    

                k = 3;
                sct_assert_level(2);
                wait();  // 2
            }
            k = 2;
            sct_assert_level(1);
            wait(); // 3
        }
    }

    // Multiple @break in for with wait()
    void break_in_for_wait3()
    {
        int k = 0;
        wait();
        
        while (1) {     // B14

            for (int i = 0; i < 3 ; i++) {          // B12
                if (in.read() > 1) {                // B11
                                                    // B10
                    for (int j = 0; j < 3; j++) {   // B9
                        
                        if (in.read() > 2) {        // B8
                            sct_assert_level(5);
                            k = 1;                  // B7
                            break;
                        }
                        sct_assert_level(4);
                        
                        k = 3;                      // B6
                        wait();   // 1
                    }
                    sct_assert_level(3);
                    
                    k = 2;                  // B5
                    wait();  // 2
                    break;                  // B4
                }
                sct_assert_level(2);

                k = 4;              // B3
                wait();  // 3
            }                       // B2   
        }                           // B1
    }
    
    // Multiple @break sequenced in for with wait()
    void break_in_for_wait4()
    {
        int k = 0;
        wait();
        
        while (1) {
            for (int i = 0; i < 3 ; i++) {
                
                if (in.read() > 1) {
                    sct_assert_level(3);
                    
                    for (int j = 0; j < 3; j++) {
                        
                        if (in.read() > 2) {
                            sct_assert_level(5);
                            k = 1;
                            break;
                        }
                        sct_assert_level(4);
                        
                        k = 2;
                        wait();   // 1
                    }
                    sct_assert_level(3);
                    k = 5;
                    break;
                }
                sct_assert_level(2);

                k = 3;
                wait();  // 2
            }
            sct_assert_level(1);

            k = 4;
            wait();  // 3
        }
    }    
 
// ---------------------------------------------------------------------------    
    
    // Loop with exit by break only, break in then branch
    void break_exit1() {
        int k = 0;
        wait();
        
        while (1) {
            for ( ; ; ) {
                wait();
                
                if (a.read()) {
                    break;
                }
                sct_assert_level(2);
                k = k + 1;
            }
            sct_assert_level(1);
            k = 1;
        }
    }    

    // Loop with exit by break only, break in else branch
    void break_exit2() {
        int k = 0;
        wait();
        
        while (1) {
            for ( ; ; ) {
                wait();         // 1
                
                if (a.read()) {
                    sct_assert_level(3);
                } else {
                    break;
                }
                k = k + 1;
                sct_assert_level(2);
            }
            wait();             // 2
            k = 1;
            sct_assert_level(1);
        }
    }    
    
    void break_exit2a() {
        int k = 0;
        wait();
        
        while (1) {
            for ( ; ; ) {
                wait();         // 1
                
                if (a.read()) {
                } else {
                    if (b.read()) break;
                    k = 1;
                }
                k = 2;
            }
            wait();             // 2
            k = 3;
            sct_assert_level(1);
        }
    }    
    
    // Loop with exit by break only, break in else branch, then branch NOT empty
    void break_exit3() {
        int k = 0;
        wait();
        
        while (1) {
            for ( ; ; ) {
                if (a.read()) {
                    k = 1;
                } else {
                    sct_assert_level(3);
                    break;
                }
                //sct_assert_level(2);
                wait();                     // 1
                
                k = k + 1;      
                if (b.read()) {
                    sct_assert_level(3);
                    k = 2;
                }
                sct_assert_level(2);
            }
            sct_assert_level(1);
            wait();                         // 2
            
            k = 1;
        }
    }    
    
    // Loop with exit by break only, break in else branch, then branch NOT empty
    void break_exit4() 
    {
        int k = 0;
        
        while (1) 
        {
            wait();         // 0
            for ( ; ; ) {
                if (a.read()) {
                    k = 1;
                    sct_assert_level(3);
                } else {
                    break;
                }
                if (b.read()) {     
                    k = 2;
                }
                for (int i = 0; i < 3; i++) {
                    if (b.read()) break;
                    sct_assert_level(3);
                }
                wait();     // 1
            }
            sct_assert_level(1);
            k = 3;
        }
    }    
    
    void break_exit4a() 
    {
        int k = 0;
        
        while (1) 
        {
            wait();         // 0
            for ( ; ; ) {
                if (a.read()) {
                    k = 1;
                    break;
                } else {
                    if (b.read()) break;
                }
                k = 2;
                wait();     // 1
            }
            sct_assert_level(1);
            k = 3;
        }
    }  
        
    // Loop with exit by break only, break in else branch, then branch NOT empty
    void break_exit5() {
        int k = 0;
        wait();
        
        while (1) 
        {
            k = 0;
            for ( ; ; ) {
                if (a.read()) {
                    k = 1;
                } else {
                    wait();
                    sct_assert_level(3);
                    if (b.read()) break;
                }
                sct_assert_level(2);
                wait();
                k = 2;
            }
            sct_assert_level(1);
            wait();
        }
    }             
    
// ---------------------------------------------------------------------------    
    
    // Conditional @break after wait
    void break_in_while_wait1()
    {
        int k = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                k = 2;
                wait();  // 1

                if (in.read() > 1)
                    break;
                i++;
            }
            sct_assert_level(1);
            k = 1;
            wait();     // 2
        }
    }
    
    // Conditional @break in inner while loop
    void break_in_while_wait2()
    {
        int k = 0;
        wait();
        
        while (1) {
            int i = 0;
            while (i < 3) {
                k = 1;
                wait();         // 1

                if (a.read()) {
                    int j = 0;
                    while (j < i) {
                        j++;
                        if (b.read()) break;
                    }
                    k = 2;
                    if (b.read()) break;
                    wait();     // 2
                }
                i++;
            }
            sct_assert_level(1);
            k = 3;
            wait();             // 3
        }
    }
    
    
    // Pass-through while loop
    void break_in_while_wait3()
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
                    if (b.read()) break;
                }
                wait();

                if (a.read()) {
                    k = 2;
                    if (b.read()) break;
                }
                i++;
            }
            sct_assert_level(1);
            k = 3;
        }
    }
    
    // While on signal with break
    void break_in_while_wait4()
    {
        int k = 0;
        wait();
        
        while (1) {

            while (a.read()) {
                k = 1;
                wait();     // 1

                if (b.read()) break;
                k = 2;
            }
            sct_assert_level(1);
            k = 3;
            wait();         // 2
        }
    }
    
    
    // While and for loops with break
    void break_in_while_for()
    {
        int k = 0;
        wait();
        
        while (1) {

            while (a.read() || b.read()) {
                if (b.read()) {
                    k = 1;
                    for (int i = 0; i < 3; i++) {
                        wait();     // 1
                        if (a.read()) break;
                    }
                    if (b.read()) break;
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
    
// ---------------------------------------------------------------------------    
    
    // do/while with @break 
    void break_in_do_while_wait1()
    {
        int k = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                wait();  // 1

                if (a.read()) break;
                k = 2;
            } while (i < 3);
            sct_assert_level(1);

            k = 1;
            wait();     // 2
        }
    }    
    
    
    void break_in_do_while_wait2()
    {
        int k = 0;
        wait();
        
        while (1) {

            int i = 0;
            do {
                i++;
                if (b.read()) {
                    break;
                } else {
                    k = 2;
                    wait(); // 1
                }                                    \
            } while (i < 3 || a.read());
            sct_assert_level(1);

            k = 1;
            wait();         // 2
        }
    }    
    
    
    void break_in_do_while_wait3()
    {
        int k = 0;
        wait();
        
        while (1) {

            do {
                int i = 0;
                do {
                    i++;
                    k = 1;
                    wait();
                    if (b.read()) break;
                } while (i < 30);

                k = 2;
                wait();
                    
            } while (a.read());
            sct_assert_level(1);

            k = 3;
            wait();         // 3
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

