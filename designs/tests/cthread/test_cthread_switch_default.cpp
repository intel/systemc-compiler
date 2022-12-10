/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// switch various code in default, empty cases
class top : sc_module
{
public:
    sc_in_clk               clk;
    sc_signal<bool>         arstn;
    sc_signal<int>          s;
    sc_signal<sc_uint<4>>   t;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(test_switch_const_default1, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_const_default2, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_const_default3, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(test_switch_const_default_empty1, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_const_default_empty2, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_const_default_empty3, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_const_default_empty4, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(test_switch_simple_default1, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_simple_default2, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_simple_default3, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(test_switch_simple_default_empty1, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_simple_default_empty2, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_simple_default_empty3, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_simple_default_empty4, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(test_switch_default1, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_default_wait1, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_default_empty1, clk.pos());
        async_reset_signal_is(arstn, false);
        SC_CTHREAD(test_switch_default_wait_empty1, clk.pos());
        async_reset_signal_is(arstn, false);
    }
    
    static const unsigned N = 2;
    
    void test_switch_const_default1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (N) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; break; 
                default: ;
            }
            wait();
        }
    }    
      
    void test_switch_const_default2()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (N-1) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; break; 
                default: ;
            }
            wait();
        }
    }    
      
    void test_switch_const_default3()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (N-2) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; break; 
                default: ;
            }
            wait();
        }
    }   

    void test_switch_const_default_empty1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (N-2) 
            {
                case 0: j = 1; break; 
                case 1: 
                default: j = 2; break;
            }
            wait();
        }
    }    
      
    void test_switch_const_default_empty2()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (N-1) 
            {
                case 0: j = 1; break; 
                case 1: 
                default: j = 2; break;
            }
            wait();
        }
    }    
    
    void test_switch_const_default_empty3()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (N-1) 
            {
                case 0:
                case 1: 
                default: j = 2; break;
            }
            wait();
        }
    }  
    
    void test_switch_const_default_empty4()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (N-2) 
            {
                case 0: 
                case 1: 
                default: j = 2; break;
            }
            wait();
        }
    }    
      
    void test_switch_simple_default1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (t.read()) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; break; 
                default: ;
            }
            wait();
        }
    }    
    
    void test_switch_simple_default2()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (t.read()) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; break; 
                default: break;
            }
            wait();
        }
    }    
    
    void test_switch_simple_default3()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (t.read()) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; j++; break; 
                default: j = 3; j--; break;
            }
            wait();
        }
    }    
    
    void test_switch_simple_default_empty1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (t.read()) 
            {
                case 0:
                case 1: j = 2; break; 
                default: ;
            }
            wait();
        }
    }    
    
    void test_switch_simple_default_empty2()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (t.read()) 
            {
                case 0:
                case 1: j = 2; break; 
                default: break;
            }
            wait();
        }
    }    
    
    void test_switch_simple_default_empty3()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (t.read()) 
            {
                case 0: 
                case 1: j = 2; j++; break; 
                default: j = 3; j--; break;
            }
            wait();
        }
    }    
    
    void test_switch_simple_default_empty4()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            switch (t.read()) 
            {
                case 0: 
                case 1: 
                default: j = 3; j--; break;
            }
            wait();
        }
    }    

    
    // Default with IF and loops
    void test_switch_default1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            wait();
            
            switch (t.read()) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; break; 
                default: 
                    if (s.read() == j) {
                        j++;
                    }
                    for (int k = 0; k < 2; k++) {
                        j--;
                        sc_uint<4> x;
                        switch (s.read()) {
                            case 1: j++; break;
                            default: j--; x = j; break;
                        }
                    }
                    break;
            }
        }
    }
    
    // Default with IF and loops with @wait()
    void test_switch_default_wait1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            wait();                                         // 1
            
            switch (t.read()) 
            {
                case 0: j = 1; break; 
                case 1: j = 2; break; 
                default: 
                    if (s.read() == j) {
                        j++;
                        wait();                             // 2
                    }
                    for (int k = 0; k < 2; k++) {
                        j--;
                        wait();                             // 3

                        sc_uint<4> x;
                        switch (s.read()) {
                            case 1: j++; break;
                            default: j--; x = j; break;
                        }
                    }
                    break;
            }
        }
    }
   
    
    // Default with IF and loops with empty cases
    void test_switch_default_empty1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            wait();
            
            switch (t.read()) 
            {
                case 0: 
                case 1: j = 2; break; 
                default: 
                    if (s.read() == j) {
                        j++;
                    }
                    for (int k = 0; k < 2; k++) {
                        j--;
                        sc_uint<4> x;
                        switch (s.read()) {
                            case 1: 
                            default: j--; x = j; break;
                        }
                    }
                    break;
            }
        }
    }
    
     // Default with IF and loops with @wait()
    void test_switch_default_wait_empty1()
    {
        int j = 0;
        wait();
        
        while(true) 
        {
            wait();                                         // 1
            
            switch (t.read()) 
            {
                case 0: 
                case 1: j = 2; break; 
                default: 
                    if (s.read() == j) {
                        j++;
                        wait();                             // 2
                    }
                    for (int k = 0; k < 2; k++) {
                        j--;
                        wait();                             // 3

                        sc_uint<4> x;
                        switch (s.read()) {
                            case 1: 
                            default: j--; x = j; break;
                        }
                    }
                    break;
            }
        }
    }
    
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start();
    return 0;
}

