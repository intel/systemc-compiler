/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include "sct_assert.h"

void wait_function() {
    wait();
}

// Function call general cases
class top : sc_module {
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> out{"out"};
    sc_signal<int> in{"in"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(test_thread0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_in_if1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_thread_in_if2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
         
        SC_THREAD(test_thread_in_for1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_thread_in_for2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_in_if3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_thread_multiple1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_multiple2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_multiple3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_params1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
          
        SC_THREAD(test_thread_params2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_thread_params3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_params4);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
         
        SC_THREAD(test_thread_params5);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
         
        SC_THREAD(test_thread_multiple_wait1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
         
        SC_THREAD(test_thread_multiple_wait2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_thread_inside_call1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false); 
        
        SC_THREAD(test_thread_inside_call2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_in_while1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_with_break1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread_with_break2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_thread_with_break3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_thread_reg_arg);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
    }
    
    // ------------------------------------------------------------------------
    
    void no_wait_method() {
    }
    
    void f1() {
        int k = 0;
        wait();
        k = 1;
    }

    void f2(int i) {
        int k = i;
        wait();
        k = i+1;
    }

    // wait() in IF
    int f3(int i) {
        if (i) {
            wait(); 
        }
        return (i+1);
    }
    
    // wait() in FOR
    void f4(int N) {
        for (int i = 0; i < N; i++) {
            wait();
        }
    }
    
    // Multiple wait()
    void f5() 
    {
        int k = 0;
        wait();     // 2

        k = 1;
        wait();     // 3
        k = 2;
    }

    // Multiple wait()
    void f5_() 
    {
        int k = 0;
        wait();     // 2

        if (in.read()) {
            k = 1;
            wait();     // 3
        }
        k = 2;
    }

    // Function with parameters
    int f6(int val) {
        wait();
        return (val-1);
    }

    bool f6_(sc_int<3> val) {
        wait();
        return val;
    }
    
    // Multiple return
    int f6__(bool val1, int val2, sc_uint<3>& val3) {
        if (val1) {
            wait();
            return val2+1;
        } else {
            val3 = val3 + val2;
            return val2+2;
        }
    }

    // Function with function call inside
    void f7() {
        int l = 0;
        f1();
        l = 1;
    }

    void f7_(int val) {
        int l = 0;
        if (val > 0) {
            f1();
        }
        l = 1;
        wait();
    }
    
    // -----------------------------------------------------------------------

    void test_meth0()
    {
        no_wait_method();
    }
    
    void test_thread0()
    {
        out = 0;
        wait();
        
        while (1) {
            out = 1;
            f1();       // 2
            out = 2;
        }
    }

    // Function call in IF
    void test_thread_in_if1()
    {
        out = 0;
        wait();
        
        while (1) {
            out = 1;
            
            if (in.read() > 1) {
                f1();  // 2
            }

            out = 2;
            wait();  // 3

            out = 3;
        }
    }
    
    // Function call in double IF
    void test_thread_in_if2()
    {
        out = 0;
        wait();
        
        while (1) {
            out = 1;
            
            if (in.read() > 1) {
                if (in.read() > 2) {
                    f1();  // 2
                }
            }

            out = 2;
            wait();  // 3
            
            out = 3;
        }
    }

    // Function call in after IF
    void test_thread_in_if3()
    {
        out = 0;
        wait();
        
        while (1) {
            out = 1;
            
            if (in.read() > 1) {
                if (in.read() > 2) {
                    out = 4;
                }
                f1();  // 2
                
            }

            out = 2;
            wait();  // 3
            
            out = 3;
        }
    }
    
    // Function call in FOR loop
    void test_thread_in_for1()
    {
        out = 0;
        wait();
        
        while (1) {
            out = 1;
            
            for (int i = 0; i < 3; i++) {
                f1();  // 2
            }

            out = 2;
            wait();  // 3

            out = 3;
        }
    }

    // Function call in double FOR loop
    void test_thread_in_for2()
    {
        out = 0;
        wait();
        
        while (1) {
            out = 1;
            
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    f1();  // 2
                }
            }

            out = 2;
            wait();  // 3

            out = 3;
        }
    }

    // Function call in WHILE
    void test_thread_in_while1()
    {
        wait();
        
        while (1) {
            int j = 0;
            
            while (j < 3) {
                f4(j);  // 2
                wait();
                j++;
            }

            wait();  // 3
        }
    }    

    void test_thread_with_break1()
    {
        wait();
        
        while (1) {
            int j = 0;
            
            while (j < 3) {
                if (in.read()) {
                    f5();      // 1,2
                    break;
                } 
                
                wait(); // 3
                j++;
            }
        }
    }    
    
    void test_thread_with_break2()
    {
        wait();
        
        while (1) {
            int j = 0;
            
            while (j < 3) {
                wait(); // 2
                
                if (in.read()) {
                    break;
                } 
                
                f3(j);  // 3
                
                j++;
            }
        }
    }    
    
    void test_thread_with_break3()
    {
        wait();
        
        while (1) {
            int j = 0;
            
            while (j < 3) {
                wait(); // 2
                
                if (in.read()) {
                    break;
                } 
                j++;
            }
        }
    }    
    
    // -----------------------------------------------------------------------

    // Function multiple calls
    void test_thread_multiple1()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f1();  // 2

            j = 2;
            f1();  // 3
        }
    }
    
    // Function multiple calls
    void test_thread_multiple2()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f2(1);  // 2

            j = 2;
            f2(2);  // 3
        }
    }
    
    // Function multiple calls
    void test_thread_multiple3()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f1();   // 1

            j = 2;
            f2(1);  // 2
            
            if (in.read()) {
                f1();   // 3
                f2(2);  // 4
            }
            
            f1();  // 5
        }
    }
    
    // Function calls with function call inside
    void test_thread_inside_call1()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f7();   // 2
            j = 2;
        }
    }

    // Function calls with function call inside
    void test_thread_inside_call2()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f7_(j);   // 2, 3
            j = 2;
        }
    }
    
    // -----------------------------------------------------------------------
    
    // Function with parameters
    void test_thread_params1()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = f3(j);   // 2
            
            wait(); // 3
        }
    }

    // Function with parameters
    void test_thread_params2()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f4(1);   // 2
            j = 2;
        }
    }

    // Function with parameters
    void test_thread_params3()
    {
        int j = 0;
        wait();
        
        while (1) {
            wait();     // 2
            j = f6(1);  // 3
        }
    }

    // Function with parameters
    void test_thread_params4()
    {
        int j = 0;
        wait();
        
        while (1) {
            bool b = f6_(j);  // 2 
            wait();    // 3
            
            out = b;
        }
    }

    // Function with parameters
    void test_thread_params5()
    {
        int j = 1;
        wait();
        
        while (1) {
            bool b = in.read();
            sc_uint<3> x = 5;
            j = f6__(b, j, x);  // 2 
            wait();    // 3
            
            out = x;
        }
    }
    
    // -----------------------------------------------------------------------
    
    // Function calls with multiple wait() inside
    void test_thread_multiple_wait1()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f5();  // 2, 3
            j = 2;
        }
    }

    // Function calls with multiple wait() inside
    void test_thread_multiple_wait2()
    {
        int j = 0;
        wait();
        
        while (1) {
            j = 1;
            f5_();  // 2, 3
            j = 2;
        }
    }    

    // -----------------------------------------------------------------------

    void test_thread_glob_func()
    {
        out = 0;
        wait();
        while (1) {
            out = 1;
            //cout << "one\n";
            f1();
            out = 2;
            //cout << "two\n";
            wait_function();
            out = 3;
            //cout << "three\n";
            f1();
        }
    }
    
// ----------------------------------------------------------------------------
    // Function call with register argument, check correct name in parameter 
    // assignment
    int f0(int val) {
        sct_assert_defined(val);
        return (val-1);
    }
    
    void test_thread_reg_arg()
    {
        int i = 0;
        wait();
        while(true) {
            f0(i);
            wait();
            
            i = i + 1;
        }
    }

};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

