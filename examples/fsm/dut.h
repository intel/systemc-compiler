/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Simple FSM: Sequence 101 detector
// The fsm_wait_style is an example of wait() commands used
//     to traverse states
// The fsm_case_style is similar to verilog coding,
//     which can also be used for this simple example.

enum states  {
    INIT, S1, S10, S101
};

struct Dut : sc_module 
{
    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};
    sc_in<sc_uint<1>> a;
    sc_out<sc_uint<1>> zs;
    sc_out<sc_uint<1>> zv;

    states cs; // current_state

    SC_CTOR(Dut) {
        SC_HAS_PROCESS(Dut);
        
        SC_CTHREAD(fsm_wait_style, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(fsm_case_style, clk.pos());
        async_reset_signal_is(rst, true);

    }
    

    void fsm_wait_style() {
        zs = 0;
        wait();                           // wait for reset de-assertion
        
        while (true) {

            while (a.read() == 0) wait(); // INIT S
            wait();                       // S1
                                          //
            while (a.read() == 0) {       //
                zs = 0;                   //
                wait();                   // S10
                if (a.read() == 1) {      //
                    zs = 1;
                    wait();               // S101
                } else {
                    break;                // When a has consecutive zeros: 00
                }
            }
            zs = 0;
        }
    }
    void fsm_case_style() {
        zv.write(0);
        cs = INIT;
        wait();

        while (true) {
            switch(cs) {
            case INIT: cs = (a.read()==1)? S1: INIT; break;
            case S1  : cs = (a.read()==0)? S10: S1; break;
            case S10 : cs = (a.read()==1)? S101: INIT; break;
            case S101: cs = (a.read()==1)? S1: S10; break;
            }
            zv.write(cs==S101);
            wait();
        }
    }

};
