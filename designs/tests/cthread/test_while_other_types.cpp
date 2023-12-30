/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// while with SC tyeps and bit/range/... operations in condition
class top : sc_module
{
public:
    sc_in<bool> clk;
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};
    sc_signal<int> out2{"out2"};
    sc_signal<int> out3{"out3"};
    sc_signal<int> out4{"out4"};
    sc_signal<int> out5{"out5"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(while_with_wait0_sc_int, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_wait0a_sc_uint, clk.pos());
        async_reset_signal_is(arstn, false);
          
        SC_CTHREAD(while_with_wait1_sc_bigint, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_wait2_sc_biguint, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_for_sc_int, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_signal_cond_sc_uint, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(while_with_signal_cond_sc_uint2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_binary_oper_sc_bigint, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_binary_oper1_sc_biguint, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_binary_oper2_sc_int, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(while_with_binary_oper3_sc_uint, clk.pos());
        async_reset_signal_is(arstn, false);
    }

    // @while with wait
    void while_with_wait0_sc_int()
    {
        wait();
        
        while (1) { // B6

            sc_int<32> i = 0;  // B5
            while (i < 3) { // B4
                wait();     // B3
                i++;
            }           // B2
        }   // B1
    }
    
    // @while with wait and global iteration counter
    void while_with_wait0a_sc_uint()
    {
        sc_uint<10> i = 0;
        wait();
        
        while (1) { 
            while (i < 3) {
                wait();     
                i++;
            }
            wait();
        }   
    }
    
    // @while with wait
    void while_with_wait1_sc_bigint()
    {
        out = 0;
        wait();
        
        while (1) {

            sc_bigint<64> i = 0;
            while (i < 3) {
                i++;
                out = 1;
                wait();     // 2
            }
            out = 2;
            wait();     // 3
        }
    }
    
    // @while with conditional wait
    void while_with_wait2_sc_biguint()
    {
        out2 = 0;
        wait();
        
        while (1) {

            sc_biguint<32> i = 0;
            while (i < 3) {
                i++;
                out2 = 1;
                wait();     // 2
                
                if (in.read() > 1) {
                    out2 = 2;
                    wait();  // 3
                }
            }
            out2 = 3;
            wait();     // 4
        }
    }
    
    // @while with inner @for 
    void while_with_for_sc_int()
    {
        out3 = 0;
        wait();
        
        while (1) {

            sc_int<20> i = 0;
            while (i < 3) {
                i++;
                out3 = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        out3 = j;
                    }
                    wait();  // 2
                }
            }
            out3 = 3;
            wait();     // 3
        }
    }

    // @while with signal condition
    void while_with_signal_cond_sc_uint()
    {
        out4 = 0;
        wait();
        
        while (1) {

            while (in.read()) {
                out4 = 1;
                wait();     // 0
            }

            out4 = 2;
            wait();         // 0
        }
    }
    
    sc_signal<sc_uint<12>> s0;
    sc_signal<sc_uint<12>> s1;
    void while_with_signal_cond_sc_uint2()
    {
        s1 = 0;
        wait();
        
        while (1) {

            while (s0.read().range(7,4) > 2) {
                s1 = 1;
                wait();     // 1
                while (s1.read().bit(3)) {
                    wait(); // 2
                }
            }

            s1 = 2;
            wait();     // 3
        }
    }

    // While with binary ||/&& operator -- BUG in real design EMC
    sc_signal<int> t0;
    void while_with_binary_oper_sc_bigint()
    {
        sc_bigint<50> b1, b2;
        int k = 0;
        wait();
        
        while (1) {             // B7
            while (b1.or_reduce() || b2.or_reduce()) {  // B6, B5
                k = 1;        // B4
                wait();
                k = 2;
            }                   // B3
            t0 = k;
            wait();             // B2, B1
        }
    }
    
    sc_signal<int> t1;
    void while_with_binary_oper1_sc_biguint()
    {
        sc_biguint<25> b1, b2;
        int k = 0;
        wait();
        
        while (1) {             
            while (b1.or_reduce()  && b2.or_reduce() ) {
                k = 1;          
                wait();
                k = 2;
            }        
            t1 = k;
            wait();             
        }
    }
    
    // While with binary ||/&& operator -- BUG in real design EMC fixed
    sc_signal<int> t2;
    void while_with_binary_oper2_sc_int()
    { 
        sc_int<32> b1, b2, b3;
        int k = 0;
        wait();     // B9
        
        while (1) {         // B8
            while ((b1.or_reduce()  || b2.or_reduce() ) && b3.or_reduce() ) {  // B7, B6, B5
                k = 1;
                wait();     // B4
                k = 2;
            }               // B3
            t2 = k;
            wait();         // B2
        }
    }
    
    sc_signal<int> t3;
    void while_with_binary_oper3_sc_uint()
    { 
        sc_uint<16> b1, b2, b3;
        int k = 0;
        wait();     
        
        while (1) { 
            while ((b1.or_reduce()  && b2.or_reduce() ) || b3.or_reduce() ) {
                k = 1;
                wait();     
                k = 2;
            }               
            t3 = k;
            wait();         
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

