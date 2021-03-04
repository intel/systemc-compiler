/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// switch with empty case(s)
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
        SC_THREAD(test_switch_empty1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty2a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty3a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty4);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty4a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty4b);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty4c);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_switch_empty4d);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_switch_empty4e);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty4f);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_switch_empty4g);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_switch_empty4i);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(test_switch_empty5);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty5a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty6);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_switch_empty6a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread);
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
            case 0: 
            case 1: k = 2; 
                    wait();
                    break; 
        }
        k = 3;
    }

    int f2(int i) 
    {
        switch (i) {
            case 0: 
            case 1: wait(); 
                    return (i+2); 
            default: return i;
        }
    }
    
    // switch in thread with wait()
    void test_switch_empty1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = j;
            switch (i) 
            {
                case 0: 
                case 1: j = 2; break;
                default: j = 3;
            }
            wait();
            i = 1;
        }
    }

    // switch with wait() in cases
    void test_switch_empty2()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            switch (i) 
            {
                case 0: 
                case 1: j = 2; wait(); break;   // 2
                default: j = 3; break;
            }
            wait(); // 3
            i = 1;
        }
    }
    
    void test_switch_empty2a()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (in.read()) 
            {
                case 0: 
                case 1: j = 2; wait(); break;   // 2
                default: j = 3; break;
            }
            wait(); // 3
        }
    }

    // switch with wait() and function call in cases
    void test_switch_empty3()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            wait(); // 1
            
            switch (i) {
                case 0: 
                case 1: j = 2; f(); break;          // 2
                // @break must be here to provide remove switch from loop stack and level up 
                default: j = 3; wait(); break;      // 3
            }
            i = 1;
        }
    }
    
    void test_switch_empty3a()
    {
        int j = 0;
        wait();     // 0
        
        while(true) 
        {
            wait(); // 1
            
            switch (in.read()) {
                case 0: 
                case 1: j = 2; f(); break;          // 2
                // @break must be here to provide remove switch from loop stack and level up 
                default: j = 3; wait(); break;      // 3
            }
            j++;
        }
    }
    
    // switch with wait() in IF inside cases
    void test_switch_empty4()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            wait(); // 1
            
            switch (i) 
            {
                case 0: 
                case 1: j = 2; 
                        if (in.read()) wait();  // 2
                        break; 
                default: j = 3; break;
            }
            i = 1;
        }
    }
    
    void test_switch_empty4a()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 1;
            wait(); // 1
            
            switch (i) 
            {
                case 0: 
                case 1: j = 2; 
                        if (in.read()) wait();  // 2
                        break; 
                default: j = 3; break;
            }
            i = 1;
        }
    }
    
    void test_switch_empty4b()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 2;
            wait(); // 1
            
            switch (i) 
            {
                case 0: 
                case 1: j = 2; 
                        if (in.read()) wait();  // 2
                        break; 
                default: j = 3; break;
            }
            i = 1;
        }
    }
    
    void test_switch_empty4c()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            wait(); // 1
            
            switch (in.read()) 
            {
                case 0: 
                case 1: j = 2; 
                        if (in.read()) wait();  // 2
                        break; 
                default: j = 3; break;
            }
            i = 1;
        }
    }
    
    // switch with wait() in FOR inside cases and non-empty default
    void test_switch_empty4d()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            wait(); // 1
            
            switch (in.read()) 
            {
                case 0: 
                case 1: j = 2; 
                        for (int k = 0; k < 2; k++) {
                            wait();  // 2
                            j++;
                        }
                        break; 
                default : j = 3; break;
            }
            i = 1;
        }
    }
    
    // switch with wait() in FOR inside cases and empty default
    void test_switch_empty4e()
    {
        while(true)                                    // B10
        {
            wait(); // 0
            
            switch (in.read())                         // B2
            {
                case 0:                                // B5
                case 1: for (int k = 0; k < 2; k++) {  // B9, B8, B6
                            wait();  // 1              // B7  
                        }
                        break;                         // B4
                default: ;                             // B3
            }
        }                                              // B1 
    }
    
    void test_switch_empty4f()
    {
        while(true) 
        {
            wait(); // 0
            
            switch (in.read())                         // B2
            {
                case 0:                                // B5
                case 1: for (int k = 0; k < 2; k++) {  // B9, B8, B6
                            wait();  // 1              // B7  
                        }
                        break;                         // B4
                case 2: ;
                default: ;                             // B3
            }
        }                                              // B1 
    }
    
    void test_switch_empty4g()
    {
        while(true)                                    
        {
            wait(); // 0
            
            switch (in.read())                         
            {
                case 0:                                
                case 1: wait();  // 1               
                        break;                         
                default: ;                             
            }
        }                                              
    }
    
    void test_switch_empty4i()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            int i = 0;
            wait(); // 1
            
            switch (in.read()) 
            {
                case 0:  j = 1; break;
                case 1: 
                default: j = 2; 
                         if (in.read()) wait();  // 2
                         break; 
            }
            i = 1;
        }
    }
    
    // switch in function with wait() 
    void test_switch_empty5()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            j = 1;
            wait(); // 1
            
            j = 2;
            if (in.read()) {
                f1(j-1);
            }
        }
    }
    
    void test_switch_empty5a()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            j = 1;
            wait(); // 1
            
            if (in.read()) {
                f1(in.read()); 
            }
        }
    }

    // switch in function with wait() 
    void test_switch_empty6()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            j = f2(j); // 1
            wait();    // 2
        }
    }
    
    void test_switch_empty6a()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            j = f2(in.read());  // 1
            wait();             // 2
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
            case 0: 
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

