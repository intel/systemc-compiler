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
    
     void single_state() {
        int i = 0;
        wait();

        while (true) {
            i = 2;
            wait();
        }
    }
     
    void not_single_state1() {
        int i = 0;
        wait();
        
        i = 1;

        while (true) {
            i = 2;
            wait();
        }
    }

    void not_single_state2() {
        int i = 0;
        wait();
        
        i = 1;
        wait();

        while (true) {
            i = 2;
            wait();
        }
    }
    
    void not_single_state3() {
        int i = 0;
        wait();
        
        if (a) {
            i = 1;
            wait();
        }

        while (true) {
            i = 2;
            wait();
        }
    }
    
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
            wait();     // #2
        }
    }
    
    void state_in_for() {
        bool b = 0;
        wait();         // #0
        
        while (true) {
            for (int i = 0; i < 3; i++) {
                b = !b;
                wait(); // #1
                
                if (a.read()) break;
            }
            wait();     // #2
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
        clk(clk_gen);
        a_mod.clk(clk);
        a_mod.rst(rst);
    }
};

int sc_main(int argc, char *argv[]) {

    B_top b_mod{"b_mod"};
    
    sc_start();
    return 0;
}

