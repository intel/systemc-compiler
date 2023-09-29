/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>

// First and last state in generated case are duplicated, for #40
SC_MODULE(dut) {

    sc_signal<bool> clk;
    sc_signal<bool> rstn;
 
    SC_CTOR(dut) {
        SC_CTHREAD(assert_thread, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_single, clk);
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_while, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_while2, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_if, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_if2, clk);
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread_fcall, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_break, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_break_single, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_wait_n, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_wait_n2, clk);
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread_wait_n3, clk);
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread_wait_n4, clk);
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread_wait_n5, clk);
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(thread_one_state, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_one_state2, clk);
        async_reset_signal_is(rstn, false);
    }

    sc_signal<bool> b;
    void assert_thread() {
        b = 0;
        SCT_ASSERT_THREAD (b == 0 && rstn, (1), b == 1, clk.pos())
        wait();

        SCT_ASSERT_THREAD (b == 0, (1), b == 1, clk.pos())

        while (true) {
            b = 1;
            wait();             // 0
        }
    }

    sc_signal<unsigned> aa;
    void thread_single() {
        aa = 0;

        while (true) {
            wait();                // 0
            aa = 1;
        }
    }

    sc_signal<unsigned> a;
    void thread() {
        a = 0;
        wait();                 // 0

        while (true) {
            a = 1;
            wait();             // 1
            a = 2;
            wait();             // 0
        }
    }

    sc_signal<bool> ready1;
    sc_signal<bool> valid1;
    void thread_while() {
        wait();                 // 0
        while (1) {

            valid1 = 1;
            wait();             // 1
            while (!ready1) {
                wait();         // 1
            }
            valid1 = 0;

            wait();             // 0
        }
    }

    sc_signal<bool> ready2;
    sc_signal<bool> valid2;
    void thread_while2() {
        valid2 = 0;
        wait();                 // 0
        while (true) {

            valid2 = 1;
            do {
                wait();         // 1
            } while (!ready2);

            int x = 42;
            wait();             // 0
        }
    }

    sc_signal<unsigned> c;
    void thread_if() {
        c = 0;
        wait();                 // 0
        while (true) {
            c = 1;
            wait();             // 1
            
            if (b) {
                c = 2;
                wait();         // 2
            } else { 
                c = 3;
                wait();         // 2
            }
            c = 4;
            wait();             // 0
        }
    }

    sc_signal<unsigned> d;
    void thread_if2() {
        d = 0;
        wait();                 // 0
        while (true) {
            if (c) {
                wait();         // 1
                d = 1;
                wait();         // 2
            } else { 
                d = 1;
                wait();         // 2
            }
            d = 2;
            wait();             // 0
        }
    }
    
    int f() {
        wait();                 
        return d.read();
    }
    
    sc_signal<unsigned> e;
    void thread_fcall() {
        e = 0;
        wait();                 // 0
        while (true) {
            e = f();            // 1
            wait();             // 0
        }
    }

    sc_signal<unsigned> r;
    void thread_break() {
        r = 0;
        wait();      
        while (true) {
            r = 1;
            while (true) {
                if (d) break;
                wait();         // 1
            }
            r = 2;
            wait();             // 0
        }
    }

    // One state, state variable not removed
    sc_signal<unsigned> p;
    void thread_break_single() {
        p = 0;
        while (true) {
            wait();             // 0
            while (true) {
                p = 1;
                if (d) break;
                wait();         // 0
            }
        }
    }
    
    sc_signal<unsigned> n;
    void thread_wait_n() {
        n = 0;
        wait();
        
        while (true) {
            n = 1;
            wait(3);            // 1 
            n = 2;
            wait();             // 0
        }
    }

    sc_signal<unsigned> m;
    void thread_wait_n2() {
        m = 0;
        
        while (true) {
            wait(3);            // 0 
            m = 1;
            wait(3);            // 1
        }
    }

    sc_signal<unsigned> mm;
    void thread_wait_n3() {
        mm = 0;
        wait();
        
        while (true) {
            mm = 1;
            wait(3);            // 1 
        }
    }

    sc_signal<unsigned> pp;
    void thread_wait_n4() {
        pp = 0;
        wait(3);
        
        while (true) {
            pp = 1;
            wait(3);            // 1 
        }
    }

    sc_signal<unsigned> kk;
    void thread_wait_n5() {
        kk = 0;
        while (true) {
            wait(3);            
            kk = 1;
        }
    }
    
    
    // One state thread is not converted to single state, that is right
    sc_signal<sc_uint<3>> q;
    void thread_one_state() 
    {
        q = 0;
        
        while (true) {
            do {
                wait();         
            } while (m.read());
            q = 1;
        }
    }    

    sc_signal<unsigned> t;
    void thread_one_state2() 
    {
        t = 0;
        wait();
        
        while (true) {
            t = 1;
            wait();                     // 1
            while (m.read()) wait();    // 1
        }
    }    
};

int sc_main(int argc, char **argv) {
    dut dut0{"dut0"};
    sc_start();
    return 0;
}
