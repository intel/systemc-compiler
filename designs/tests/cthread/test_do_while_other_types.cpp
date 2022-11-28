/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// do/while with SC data type counters
class top : sc_module
{
public:
    sc_in_clk clk;
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> out{"out"};
    sc_signal<int> out2{"out2"};
    sc_signal<int> out3{"out3"};
    sc_signal<int> out4{"out4"};
    sc_signal<int> out5{"out5"};
    sc_signal<int> out6{"out6"};
    sc_signal<int> in{"in"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(dowhile_with_wait0_sc_int, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(dowhile_with_wait1_sc_uint, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(dowhile_with_wait2_sc_bigint, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(dowhile_with_for_sc_biguint, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(dowhile_with_signal_cond, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(complex0, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(complex1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(complex2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(break_in_if, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(break_in_if2, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_METHOD(break_in_if2_m);
        sensitive << in << s;
    }

    // @do..while with wait
    void dowhile_with_wait0_sc_int()
    {
        out = 0;
        wait();
        
        while (1) {             // B7

            sc_int<11> i = 0;          // B6
            do {
                out = 1;        // B4    
                wait();  // 2
                i++;
            } while (i < 3);    // B3, B5
            
            out = 2;            // B2, B1
        }
    }
    
      // @while with wait
    void dowhile_with_wait1_sc_uint()
    {
        out2 = 0;
        wait();
        
        while (1) {

            sc_uint<32> i = 0;
            do {
                i++;
                out2 = 1;
                wait();     // 2
            } while (i < 3);
            out2 = 2;
            wait();     // 3
        }
    }
    
    // @while with conditional wait
    void dowhile_with_wait2_sc_bigint()
    {
        out3 = 0;
        wait();
        
        while (1) {

            sc_bigint<64> i = 0;
            do {
                i++;
                out3 = 1;
                wait();     // 2
                
                if (in.read() > 1) {
                    out3 = 2;
                    wait();  // 3
                }
            } while (i < 3);
            out3 = 3;
            wait();     // 4
        }
    }
    
    // @while with inner @for 
    void dowhile_with_for_sc_biguint()
    {
        out4 = 0;
        wait();
        
        while (1) {

            sc_biguint<32> i = 0;
            do {
                i++;
                out4 = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        out4 = j;
                    }
                    wait();  // 2
                }
            } while (i < 3);
            out4 = 3;
            wait();     // 3
        }
    }

    // @while with signal condition
    void dowhile_with_signal_cond()
    {
        out5 = 0;
        wait();
        
        while (1) {

            do {
                out5 = 1;
                wait();     // 2
            } while (in.read());

            out5 = 2;
            wait();     // 3
        }
    }
    
    void complex0()
    {
        wait();
        
        while (1) {

            sc_uint<4> i = 0;
            do {
                i++;
                if (i < 3) continue;
                
            } while (i < 4);
            sct_assert_level(1);

            wait();         // 2
        }
    }
    
    void complex1()
    {
        wait();
        
        while (1) {

            sc_uint<4> i = 0;
            do {
                i++;
                if (i > 4) continue;
            } while (i < 1);
            sct_assert_level(1);   

            i = 0;
            do {
                i++;
                if (in.read()) break;
                wait();     // 1
            } while (i < 3);
            sct_assert_level(1);   
            
            wait();         // 2
        }
    }
    
    void complex2()
    {
        out6 = 0;
        wait();
        while (1) {

            sc_uint<4> i = 0;
            do {
                i++;
                if (i > 3)
                    break;
                if (i > 4)
                    continue;

                i++;
            } while (i < 1);
            sct_assert_level(1);   

            do {
                i ++;
                out6 = i;
            } while (i < 5);
            sct_assert_level(1);   

            i = 0;
            do {
                i ++;
                out6 = i;

                if (in.read())
                    break;

                wait();   // 1

            } while (i < 3);
            sct_assert_level(1);   

            do {
                wait();   // 2
            } while (in.read());
            sct_assert_level(1);   
        }
    }
    
    sc_signal<sc_uint<4>> s;
    void break_in_if()
    {
        int k = 0;
        wait();
        
        while (1) {
            sc_uint<4> i = s.read();
            
            do {
                i++;
                if (in.read()) {
                    if (s.read()) break;
                } else {
                    k = 1;
                }
                wait();     // 1
                
            } while (i < 3);
            sct_assert_level(1);   

            wait();         // 2
        }
    }
    
    void break_in_if2()
    {
        int k = 0;
        wait();
        
        while (1) {
            sc_uint<4> i = s.read();
            
            do {
                i++;
                if (in.read()) {
                    if (s.read()) 
                        break;
                    else 
                        k = 1;
                }
                wait();     // 1
                
            } while (i < 3);
            sct_assert_level(1);

            wait();         // 2
        }
    }
    
    void break_in_if2_m()
    {
        int k;
        int i = 0;
        do {
            i++;
            if (in.read()) 
                if (s.read())       // B4
                    break;          // B3
        } while (i < 3);            // B2
        
        sct_assert_level(0);        // B1
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

