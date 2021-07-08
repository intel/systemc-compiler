/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// FOR loop in IF and FOR loop with IF inside
class A : public sc_module {
public:

    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};
    
    SC_CTOR(A) 
    {
        SC_CTHREAD(for_in_if1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(for_in_if2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(for_in_if3, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(for_in_if_bug, clk.pos());
        async_reset_signal_is(rst, true);
        
        
        SC_CTHREAD(for_with_if1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(for_with_if2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(for_with_if3, clk.pos());
        async_reset_signal_is(rst, true);
    }
    
    static const unsigned BLOCK_NUM = 3;
    static const unsigned TIME_CNTR_WIDTH = 3;
    
    sc_signal<bool>     sleep_enable{"sleep_enable"};
    sc_signal<bool>     block_access[BLOCK_NUM];
    sc_signal<bool>     mem_active[BLOCK_NUM];
    
    sc_signal<sc_uint<TIME_CNTR_WIDTH> >    sleep_time;
    sc_signal<sc_uint<TIME_CNTR_WIDTH> >    wakeup_time;
    
    void for_in_if1() 
    {
        int k = 0;
        unsigned sleepTime;
        wait();
        
        while (true) {
            sleepTime = sleep_time.read();
            if (sleep_enable)
            {                                               
                for (unsigned i = 0; i < sleepTime; i++) {  
                    wait();     // 1
                }               
                k = 1;      
            }
            k = 2;      
            
            wait();     // 2
        }
    }
    
    void for_in_if2() 
    {
        wait();
        
        while (true) {
            int m = 1;
            if (wakeup_time.read() > m+1)
            {         
                int j = 42;
                for (; j < wakeup_time.read(); j--) {  
                    m++;
                    wait();     // 1
                }               
                sleep_time = m;
            }
            wait();     // 2
        }
    }
    
    void for_in_if3() 
    {
        sc_uint<4> arr[4];
        for (int i = 0; i < 4; i++) {  
            arr[i] =i;
        }               
        wait();
        
        while (true) {
            if (arr[sleep_time.read()])
            { 
                wait();         // 1
            } else {
                for (int i = 0; i < 4; i++) {  
                    wait();     // 2
                    arr[i] += 1;
                }
            }
        }
    }
    
    
    // Loop with wait(), BUG in real design -- fixed
    void for_in_if_bug() 
    {
        int k = 0;
        wait();
        
        while (true) {
            bool blockFlags_flat[BLOCK_NUM];
            
            sc_uint<TIME_CNTR_WIDTH> wakeupTime = wakeup_time;
            for (unsigned i = 0; i < wakeupTime; i++) {
                wait();                     // 1
            }
            
            if (sleep_enable)
            {
                for (int i = 0; i < BLOCK_NUM; i++) {
                    blockFlags_flat[i] = block_access[i];
                    if (blockFlags_flat[i]) mem_active[i] = 0;
                }

                sc_uint<TIME_CNTR_WIDTH> sleepTime = sleep_time;
                for (unsigned i = 0; i < sleepTime; i++) {
                    wait();                 // 2
                }
                k = 1;
            }
            k = 2;
            
            wait();                         // 3
        }
    }
    
// --------------------------------------------------------------------------

    sc_signal<int> s;
    
    void for_with_if1() 
    {
        sc_int<8> arr2[4][5];
        wait();
        
        while (true) {
            int k = 0;
            for (int i = 0; i < 4; i++) {  
                if (!s.read()) {
                    wait();    
                    k = 1;
                }
                arr2[i][i+1] += s.read();
                wait();
            }
            
        }
    }
    
    void for_with_if2() 
    {
        wait();
        
        while (true) {
            
            for (int i = 0; i < 4; i++) {  
                while (s.read()) wait();  // 1
                
                wait();                     // 2

                if (i > 2) {
                    for (int j = 0; j < 2; j++) {  
                        wait();             // 3
                    }
                }
            }
        }
    }
    
    void for_with_if3() 
    {
        int l = 1;
        s = 0;
        wait();
        
        while (true) {
            
            for (int i = 0; i < 4; i++) { 
                if (l < s.read()) l++;
                wait();                     // 1

                if (l > s.read()) {
                    l--;
                } else {
                    wait();                 // 2
                }
            }
            
            for (int i = 0; i < 3; ++i) {
                s = l;
                wait();                     // 3
            }
        }
    }
    
    
};

class B_top : public sc_module {
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    sc_clock clk_gen{"clk", sc_time(1, SC_NS)};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        clk (clk_gen);
        a_mod.clk(clk);
        a_mod.rst(rst);
        
        //SC_HAS_PROCESS(B_top);
        //SC_CTHREAD(reset_proc);
    }
    
    /*void reset_proc() {
        rst = 1;
        wait();
        
        rst = 0;
    }*/
};

int sc_main(int argc, char *argv[]) {

    B_top b_mod{"b_mod"};
//    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

