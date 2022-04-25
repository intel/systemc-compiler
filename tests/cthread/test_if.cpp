/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <sct_assert.h>

// if general cases
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    static const bool   CONST_A = 1;
    
    enum {enumVal0 = 0, enumVal1 = 1};
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_METHOD(tmp);
        sensitive << in;
        
        SC_METHOD(tmp1);
        sensitive << in;

        SC_METHOD(tmp2);
        sensitive << in;

        SC_METHOD(tmp3);
        sensitive << in;
        
        SC_METHOD(empty_if);
        sensitive << in;

        SC_THREAD(variable_read_in_binaryop);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(smem_if_binary_const);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_METHOD(simple_no_wait);
        sensitive << in << out;
        
        SC_THREAD(simple_wait);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(if_stmt_wait0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(if_stmt_wait1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
          
        SC_THREAD(if_stmt_wait2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(if_stmt_wait2a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(if_stmt_wait3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(if_stmt_wait_for0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(if_stmt_wait_for1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(if_stmt_wait_for2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(if_stmt_wait_for2a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(if_stmt_wait_for2b);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(if_stmt_const_prop1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(if_stmt_const_prop2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    void tmp() {
        bool b1 = in;
        bool b2 = in;
        
        if (b1 && b2) {
            int i = 1;
        }
    }
    
    void tmp1() {
        bool b1 = in;
        bool b2 = in;
        
        if (b1 && int(b2)) {
            int i = 1;
        }
    }
    
    void tmp2() {
        bool b1 = in;
        int i = 0;
        i++;
        
        if (b1 && i) {        // This IF removed
            int i = 1;  
        }
        sct_assert_const(i == 1);
        
        i = 0;
        i++;
        if (b1 && i) {
            int i = 2;
        }
        sct_assert_const(i == 1);
    }
    
    void tmp3() {
        int n;
        n = 1;
        
        sct_assert_read(n, false);
    }

    // Empty IFs removed
    sc_signal<int> s1;
    void empty_if() {
        bool b1 = in;
        int i = 0;
        
        if (b1) {
            int j;
        }
        
        if (i) {      
            int k;
        }

        if (b1 && in.read()) {
        }
        s1 = i;
    }
    
// ---------------------------------------------------------------------------    
    
    // BUG in real design
    // No register generated for variable used in right part of binary &&/||
    void variable_read_in_binaryop() {
        bool closeWaLine = 0;
        wait();
        
        while (true) {
            bool updateLine = in;
            
            // No IF generated as condition is always false
            if (!updateLine && closeWaLine) {
                closeWaLine = 0;
            }
            
//            if (in) {
//                closeWaLine = 1;
//            }
            wait();
        }
    }
    
    // BUG from real design in fullAccessPort.h:640
    void smem_if_binary_const() {
        bool b = 0;
        wait();
        
        while (true) {          // B5
            if (CONST_A) {      // B4
                b = !b;         // B3
            }
            
            wait();             // B2
        }                       // B1
    }    
    void simple_no_wait() 
    {
        int k = 0;              // B7
        
        if (in.read()) {        
            k = 1;              // B6

            if (out.read()) { 
                k = 2;          // B5
            } else {
                k = 3;          // B4
            }

        } else {
            if (out.read()) {   // B3
                k = 4;          // B2
            }
        }

        k = 6;                  // B1    
    }       

    
    void simple_wait() 
    {
        int k = 0;
        wait();          // 0 
        
        while (true) {              // B9
            if (in.read()) {        // B8
                k = 1;              // B7
                wait();  // 1
                
                if (out.read()) { 
                    k = 2;          // B6
                    wait(); // 4 | 2
                } else {
                    k = 3;          // B5
                }
                
            } else {
                if (out.read()) {   // B4
                    k = 4;          // B3
                    wait();  // 2 | 3
                }
            }
            
            wait();  // 3 | 4       // B2
            k = 6;
        }                           // B1
    }

    void if_stmt_wait0()
    {
        int k = 0;
        wait();
        
        while (true) {
            if (in.read()) {
                k = 1;
                wait();  // 2
            }
            
            k = 2;
            wait();  // 3
            
            k = 3;
        }
    }
    
    void if_stmt_wait1()
    {
        int k = 0;
        wait();
        
        while (true) {
            k = 1;
            wait();  // 2

            if (in.read()) {
                k = 2;
                wait();  // 3
                k = 3;
            }
        }
    }
    
    void if_stmt_wait2()
    {
        int k = 0;
        wait();
        
        while (true) {
            if (in.read()) {
                k = 1;
                wait();  // 2
                k = 2;
                
            } else {
                k = 3;
                wait();  // 3
                k = 4;
            }
        }
    }
    
    void if_stmt_wait2a()
    {
        int k = 0;
        wait();
        
        while (true) {
            if (in.read()) {
                k = 1;
                wait();  // 2
                k = 2;
                
            } else {
                k = 3;
                wait();  // 3
                
                if (out.read()) {
                    k = 4;
                }
            }
        }
    }
    
    void if_stmt_wait3()
    {
        int k = 0;
        wait();
        
        while (true) {
            if (in.read()) {
                k = 1;
                wait();  // 1
                
                if (out.read()) {
                    k = 2;
                    wait();  // 2
                } else {
                    k = 3;
                }
                
            } else {
                if (out.read()) {
                    if (out.read() == in.read()) {
                        k = 4;
                        wait();  // 3
                        k = 5;
                    }
                }
            }
            
            wait();  // 4
            k = 6;
        }
    }

    // IF statement with wait() and FOR without wait()
    void if_stmt_wait_for0()
    {
        int k = 0;
        wait();
        
        while (true) {
            
            for (int i = 0; i < 3; i++) {
                k++;
            }
            
            if (in.read()) {
                k = 1;
                wait();  // 1
            }

            for (int i = 0; i < 3; i++) {
                k--;
            }

            if (in.read()) {
                k = 2;
                wait();  // 2
            }
            
            wait();  // 3
            k = 3;
        }
    }

    // IF statement with wait() and FOR without wait() inside the IF
    void if_stmt_wait_for1()
    {
        int k = 0;
        wait();
        
        while (true) {
            
            if (in.read()) {
                for (int i = 0; i < 3; i++) {
                    k++;
                }
                k = 1;
                wait();  // 1
                
                if (out.read()) {
                    for (int j = 0; j < 3; j++) {
                        k--;
                    }
                } else {
                    k = 3;
                    wait(); // 2
                }
                k = 2;
                wait(); // 3
                k = 5;
                        
            } else {
                k = 4;
                wait(); // 4
                k = 6;
            }
        }
    }

    // IF statement with wait() inside of FOR without wait()
    void if_stmt_wait_for2()
    {
        int k = 0;
        wait();
        
        while (true) {                      // B8
                                            // B7
            for (int i = 0; i < 3; i++) {   // B6
                if (in.read()) {            // B5
                    k = 1;                  // B4
                    wait();  // 1
                }
                
                wait(); // 2                // B3
            }                               // B2
        }                                   // B1
    }
    
    void if_stmt_wait_for2a()
    {
        int k = 0;
        wait();
        
        while (true) {
        
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (in.read()) {
                        k = 1;
                        wait();  // 1
                    }

                    wait(); // 2
                }
            }
            
            wait();  // 3
        }
    }
    
    void if_stmt_wait_for2b()
    {
        int k = 0;
        wait();
        
        while (true) {
            
            for (int i = 0; i < 3; i++) {
                if (in.read()) {
                    k = 1;
                    wait();  // 1
                }
            
                for (int j = 0; j < 3; j++) {
                    if (out.read()) {
                        k = 2;
                    } else {
                        k = 3;
                        wait(); // 2
                    }
                    wait(); // 3
                }
                
                wait(); // 4
            }
        }
    }
    
    void if_stmt_const_prop1()
    {
        int k = 0;
        wait();
        
        while (true) {
            
            if (true) {
                k = 1;
            } else {
                k = 2;
            }
            
            k = 3;
            wait();
        }
    }    
    
    void if_stmt_const_prop2()
    {
        int k = 0;
        wait();
        
        while (true) {
            
            if (false) {
                k = 1;
            } else {
                k = 2;
            }
            
            k = 3;
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


