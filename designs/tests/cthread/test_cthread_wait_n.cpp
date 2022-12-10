/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// wait(N) general cases
SC_MODULE(test_mod) {
    
    sc_in<bool>     clk{"clk"}; 
    sc_signal<bool> rstn{"rstn"};
    sc_signal<bool> a{"a"};

    SC_CTOR(test_mod) {
        
        SC_CTHREAD(wait_n_reset_decl, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(one_wait_n, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread0, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_waitn_first, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_waitn_no_reset, clk.pos());
        
        SC_CTHREAD(thread_waitn_first_cond, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread_waitn_cond, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread1a, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread1b, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread1c, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread1d, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread4_no_waitn, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(wait_n_const, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(wait_n_var, clk.pos());
        async_reset_signal_is(rstn, false);
         
        SC_CTHREAD(wait_n_calc, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(wait_n_calc_if, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(wait_n_calc_for, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(three_wait, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(four_wait, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(cntr_name_conflict, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(while_wait, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(wait_1, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void wait_n_reset_decl() 
    {
        unsigned k = 1;
        unsigned i = k + 1;
        wait();     

        while (1) {
            wait(2);  
        }
    }
    
    void one_wait_n () {
        sc_uint<2> i = 0;
        wait();
        while (true) {
            wait();
            i++;
            wait(2);
        }
    }

    void thread0 () {
        while (1) {
            wait();
            wait(1);
            wait(2);
            wait(3);
        }
    }

    sc_signal<sc_uint<4>> usig{"usig"};

    void thread_waitn_first () {
        usig = 0;
        while (1) {
            wait(3); 
            usig = 1;
        }
    }
    
    void thread_waitn_no_reset () {
        while (1) {
            usig = 1;
            wait(); 
        }
    }

    
    void thread_waitn_first_cond () {
        int m = 0;
        usig = 0;
        while (1) {
            if (m) {
                wait(4); 
            } else {
                wait(5); 
            }
            usig = 1;
            wait();
        }
    }
    
    void thread_waitn_cond () {
        usig = 0;
        while (1) {
            wait();
            usig = 1;
            if (a.read()) {
                wait(3); 
            } else {
                wait(5); 
                usig = 2;
                wait(2); 
            }
        }
    }
    
    void thread1a () {
        wait();
        while (1) {
            wait(3); 
        }
    }

    void thread1b () {
        wait();
        while (true) {
            int i = 0;
            wait(3);
        }
    }

    void thread1c () {
        wait();
        while (true) {
            wait(3);
            int i = 0;
        }
    }
    
    void thread1d () {
        int i = 1;
        wait();
        while (1) {
            wait(3); 
        }
    }

    void thread2 () {
        while (1) {
            wait(2*2);
            wait(1 + 1 + 1);
        }
    }

    void thread3 () {

        int n;

        while (1) {
            wait();     // 0
            n = 2;
            wait(n);    // 1
            wait(n+1);  // 2
        }
    }

    void thread4_no_waitn() {
        while (1) {
            wait();
            for (size_t i = 0; i < 3; ++i) {
                wait();
            }
        }
    }

    void wait_n_const () {
        const unsigned n = 2;
        wait();

        while (1) {
            wait(n);
        }
    }
    
    void wait_n_var () {
        unsigned n = 0;
        wait();

        while (1) {
            n = 2;
            wait(n);
        }
    }

    void wait_n_calc() 
    {
        unsigned n;
        wait();     // 0

        while (1) {
            n = 2;
            wait(n-1);    // 1
            n++;
            unsigned m = 1;
            wait(n+m);  // 2
        }
    }

    // wait(n) in IF
    void wait_n_calc_if() 
    {
        unsigned n;
        wait();     

        while (1) {
            n = 2;
            if (a.read()) wait(n++);    
            
            wait();  
        }
    }

    // wait(n) in FOR
    void wait_n_calc_for() 
    {
        unsigned n = 3;
        wait();     

        while (1) {
            for (int i = 0; i < 2; i++) {
                wait(n);    
            }
        }
    }
    // Four wait, check state variable width
    void three_wait() 
    {
        wait();     // 0

        while (1) {
            wait();
            wait();
            wait();
        }
    }
    
    // Four wait, check state variable width
    void four_wait() 
    {
        wait();     // 0

        while (1) {
            wait();
            wait();
            wait();
            wait();
        }
    }
    
    // WAIT_N counter name conflict
    bool cntr_name_conflict_WAIT_N_COUNTER;
    sc_signal<sc_uint<3>> cntr_name_conflict_WAIT_N_COUNTER_next;
    void cntr_name_conflict() 
    {
        cntr_name_conflict_WAIT_N_COUNTER = 0;
        cntr_name_conflict_WAIT_N_COUNTER_next = cntr_name_conflict_WAIT_N_COUNTER;
        wait();     // 0

        while (1) {
            cntr_name_conflict_WAIT_N_COUNTER = !cntr_name_conflict_WAIT_N_COUNTER;
            cntr_name_conflict_WAIT_N_COUNTER_next = cntr_name_conflict_WAIT_N_COUNTER;
            wait(3);
        }
    }
    
    void while_wait() 
    {
        wait();     // 0

        while (1) {
            while (a) wait(3);
            wait();
        }
    }
    

    void wait_1() 
    {
        unsigned nn = 1;
        wait();     

        while (1) {
            wait(nn);
        }
    }

};


int sc_main(int argc, char **argv) 
{
    sc_clock clk{"clk", 1, SC_NS};
    test_mod tmod{"tmod"};
    tmod.clk(clk);
    sc_start();
    return 0;
}
