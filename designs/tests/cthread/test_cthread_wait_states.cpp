/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Store states at wait() checking manually
class A : public sc_module {
public:

    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};
    
    sc_signal<bool> a{"a"};
    
    SC_CTOR(A) {
        SC_HAS_PROCESS(A);
        
        SC_CTHREAD(single_state, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(not_single_state1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(not_single_state2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(not_single_state3, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(state_in_if, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(state_in_for, clk.pos());
        async_reset_signal_is(rst, true);
    }
    
    sc_signal<int> t0;
    void single_state() {
        int i = 0;
        wait();

        while (true) {
            i = 2;
            t0 = i;
            wait();
        }
    }
    
    sc_signal<int> t1;
    void not_single_state1() {
        int i = 0;
        wait();
        
        i = 1;

        while (true) {
            i = 2;
            t1 = i;
            wait();
        }
    }

    sc_signal<int> t2;
    void not_single_state2() {
        int i = 0;
        wait();
        
        i = 1;
        wait();

        while (true) {
            i = 2;
            t2 = i;
            wait();
        }
    }
    
    sc_signal<int> t3;
    void not_single_state3() {
        int i = 0;
        wait();
        
        if (a) {
            i = 1;
            wait();
        }

        while (true) {
            i = 2;
            t3 = i;
            wait();
        }
    }
    
    sc_signal<int> t4;
    void state_in_if() {
        bool b = 0;
        wait();         // #0
        while (true) {
            b = !b;
            if (b) {
                int i = 1;
                wait(); // #1
            }
            int j = 2;
            t4 = b;
            wait();     // #2
        }
    }
    
    sc_signal<int> t5;
    void state_in_for() {
        bool b = 0;
        wait();         // #0
        
        while (true) {
            for (int i = 0; i < 3; i++) {
                b = !b;
                wait(); // #1
                
                if (a.read()) break;
            }
            t5 = b;
            wait();     // #2
        }
    }    
    
};

class B_top : public sc_module {
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     rst{"rst"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.rst(rst);
    }
};

int sc_main(int argc, char *argv[]) {

    sc_clock clk{"clk", sc_time(1, SC_NS)};
    B_top b_mod{"b_mod"};
    b_mod.clk(clk);
    sc_start();
    return 0;
}

