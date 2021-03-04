/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// switch general tests
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
        SC_THREAD(test_switch1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_switch2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch4);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch5);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch6);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    void f() 
    {
        int k = 0;
        wait();
        k = 1;
    }
    
    void f1(int i) 
    {
        int k;
        switch (i) {
            case 0: k = 1; 
                    wait();
                    break;
            case 1: k = 2; 
                    break; 
        }
        k = 3;
    }

    int f2(int i) 
    {
        switch (i) {
            case 0: return (i+1);
            case 1: wait(); 
                    return (i+2); 
            default: return i;
        }
    }
    
    // switch in thread with wait()
    void test_switch1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = j;
            switch (i) 
            {
                case 0: j = 1; break;  
                case 1: j = 2; break;
                default: j = 3;
            }
            wait();
            i = 1;
        }
    }

    // switch with wait() in cases
    void test_switch2()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            switch (i) 
            {
                case 0: j = 1; break;  
                case 1: j = 2; wait(); break;   // 2
                default: j = 3; 
            }
            wait(); // 3
            i = 1;
        }
    }

    // switch with wait() and function call in cases
    void test_switch3()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            wait(); // 2
            
            switch (i) {
                case 0: j = 1; break;  
                case 1: j = 2; f(); break;          // 3
                // @break must be here to provide remove switch from loop stack and level up 
                default: j = 3; wait(); break;      // 4
            }
            i = 1;
        }
    }
    
    // switch with wait() in IF and FOR inside cases
    void test_switch4()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            wait(); // 2
            
            switch (i) 
            {
                case 0: j = 1; 
                        for (int k = 0; k < 2; k++) {
                            wait(); // 4
                            j++;
                        }  
                        break;
                case 1: j = 2; 
                        if (in.read()) wait();  // 3
                        break; 
                default: j = 3;      
            }
            i = 1;
        }
    }
    
    // switch in function with wait() 
    void test_switch5()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            j = 1;
            wait(); // 2
            
            j = 2;
            if (in.read()) {
                f1(j);
            }
        }
    }

    // switch in function with wait() 
    void test_switch6()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            j = f2(j); // 2
            wait();    // 3
        }
    }
    
    void test_thread()
    {
        out = 0;
        wait();
        while (1) {

            switch (in.read()) {
            case 0:
            case 1: out = 10;
            break;

            case 2: {
                out = 10;
                out = 11;
            } break;
            case 3:
            default:
                out = 13;
                break;
            }

            cout << "tick\n";

            switch (in.read()) {
            case 0: out = 1;
            break;

            case 1:
                wait();     // 1
                out = 2;
                wait();     // 2
                break;

            default:
                wait();     // 3
                break;
            }

            wait();         // 4
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

