/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// for loop with and w/o wait() general cases
class top : sc_module
{
public:
    sc_in_clk clk;
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(for_stmt_no_wait1, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_stmt_no_wait2, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_stmt_no_wait3, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_stmt_no_wait4, clk.pos());
        async_reset_signal_is(arstn, false);

        
        SC_CTHREAD(for_stmt_wait0, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_stmt_wait1, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_stmt_wait2, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_stmt_wait3, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(for_stmt_wait4, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(for_stmt_wait_noiter, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_multi_wait1, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(for_multi_wait2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(for_multi_wait3, clk.pos());
        async_reset_signal_is(arstn, false);
    }

    void for_stmt_no_wait1()
    {
        int k = 0;
        wait();
        
        while (true) {
            
            for (int i = 0; i < 2; i++) {
                k = 1;
            }
            
            k = 2;
            wait();
        }
    }
    
    void for_stmt_no_wait2()
    {
        wait();
        
        while (true) {
            
            int k = 0;
            for (int i = 0; i < 4; i++) {
                k--;
            }
            sct_assert_const(k == -4);
            
            wait();

            for (int i = 0; i < 10; i++) {
                if (in) {
                    for (int j = 0; j < 4; j++) {
                        k++;
                    }
                }
            }
        }
    }
    
    // No wait() for with break
    void for_stmt_no_wait3()
    {
        wait();
        
        while (true) {
            
            int k = 11;
            for (int i = 0; i < 4; i++) {
                if (in.read() != k) {
                    break;
                }
                k *= 2;
            }
            wait();
        }
    }
    
    // No wait() for with continue
    void for_stmt_no_wait4()
    {
        int n = 1;
        unsigned m = 0;
        wait();
        
        while (true) {
            
            for (int i = 0; i < 4; i++) {
                if (in.read()) {
                    n += 1;
                } else {
                    continue;
                }
                m += n;
            }
            
            wait();
            
            m = 0;
        }
    }
    
// ----------------------------------------------------------------------------
    
    void for_stmt_wait0()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;                          // B6
            wait();     // 1
            
            for (int i = 0; i < 2; i++) {   // B5
                k = 2;                      // B4
                wait();   // 2
            }                               // B3
            k = 3;                          // B2
        }
    }
    
    void for_stmt_wait1()
    {
        int k = 0;
        wait();
        
        while (true) {
            for (int i = 0; i < 2; i++) {
                k = 1;
                wait();
            }
            k = 2;
        }
    }

    void for_stmt_wait2()
    {
        int k = 0;
        wait();
        
        while (true) {
            for (int i = 0; i < 2; i++) {
                k = 1;
                wait();
            }
            k = 2;
            wait();
            
            k = 3;
        }
    }
    
    // Double loops with break
    void for_stmt_wait3()
    {
        int k = 0;
        wait();
        
        while (true) {
            for (int i = 0; i < 2; i++) {
                k = 1;
                for (int j = 0; j < 3; j++) {
                    k = 2;
                    wait();     // 1
                }
                if (in) break;
            }
            k = 3;
            wait();             // 2
        }
    }
    
    // Double loops with continue
    void for_stmt_wait4()
    {
        int k = 0;
        wait();
        
        while (true) {
            for (int i = 0; i < 2; i++) {
                wait();
                
                if (in.read() == 42) continue;

                for (int j = 0; j < 3; j++) {
                    k++;
                }
            }
        }
    }
    
    // For with wait() no iteration
    void for_stmt_wait_noiter()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;
            for (int i = 0; i < 0; i++) {
                k = 2;
                wait();
            }
            k = 3;
            wait();
        }
    }
    
// ---------------------------------------------------------------------------
    
    void for_multi_wait1()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;
            for (int i = 0; i < 10; i++) {
                k = 2;
                wait();         // 1
                
                k = 3;
                wait();         // 2
                
                if (in) {
                    k = 4;
                    wait();     // 3
                }
            }
            wait();             // 4
        }
    }
    
    void for_multi_wait2()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;
            wait();             // 1
            
            while (!in) {
                wait();         // 2

                for (int i = 0; i <3; ++i) {
                    k = 2;
                    wait();     // 3
                }
            }
        }
    }    

    void for_multi_wait3()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;
            wait();             // 1
            
            do {
                for (int i = 0; i <3; ++i) {
                    k = 2;
                    wait();     // 2
                }

                wait();         // 3
                
                if (out) {
                    wait();     // 4
                    k = 3;
                }

            } while (in.read() != 42);
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

