/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// if with complex condition with constant LHS/RHS
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int>  out{"out"};
    sc_signal<int>  in{"in"};
    
    sc_signal<bool> s;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(const1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(const_wait1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(compl_if, clk.pos());
        async_reset_signal_is(arstn, false);
    }

// -----------------------------------------------------------------------

    // Non-wait if with complex condition
    sc_signal<int> s1;
    sc_signal<int> s2;
    sc_signal<int> s3;

    void const1()
    {
        const bool A = 0;
        const bool B = 1;
        s1 = 0;
        wait();

        while (true) 
        {
            if (A || s.read()) {
                s1 = 1;
            }
            if (s.read() || A) {
                s1 = 2;
            }
            if (s.read() && B && s.read()) {
                s1 = 3;
            }
            if (A || B && s1.read() || (A && s2.read())) {
                s1 = 4;
            }
            if (s1.read() && B && (s2.read() || (!s3.read() && A && B))) {
                s1 = 5;
            }
            if ((s1.read() && s2.read() || B && s3.read() && (!A && !B))) {
                s1 = 6;
            }
            if ((s1.read() || s2.read() && !B) || A || s3.read()) {
                s1 = 7;
            }
            wait();
        }
    }     
     
    // wait inside if with complex condition
    sc_signal<int> s4;
    void const_wait1()
    { 
        const bool A = 0;
        const bool B = 1;
        s4 = 0;
        wait();                         

        while (true)                    
        {
            if (s.read() || A) {     
                s4 = 1;
                wait();             // 1
            }
            if (s.read() && A) {     
                s4 = 2;
                wait();             
            }
            if (A && s.read() && B || s.read() == 1) {
                s4 = 3;
                wait();             // 2
            }

            wait();                 // 3
            
            if ((B && s1.read() && A) && s2.read() == 2 && !(s3.read() || false)) {
                s4 = 4;             
                wait();             
            }
            if ((s1.read() || !(s2.read() && B) && s3.read() < 3)) {
                s4 = 5;             
                wait();             // 4
            }
        }
    }
    
// -----------------------------------------------------------------------
    
    int dummy() {
        return 0;
    }

    void compl_if()
    {
        out = 0;
        while (1) {
            wait();
            if (in.read()) {
                for (size_t i = 0; i < 3; ++i) {
                    dummy();
                    if (in.read()) {
                        int ii = 1;
                        // do..while not supported yet
                        //do { ii ++; } while (ii < 3);
                    }
                }
            }

            if (1 == 1) {
                out = -1;
            }

            if (2 * 2 != 4) {
                out = -2;
            }

            if (in.read()) {
                out = 0;
            }

            if (in.read() > 2) {
                out = 1;
            } else {
                out = 2;
            }

            if (in.read() + in.read() > 2)
                ;
            else if (in.read() < 0) {
                out = 3;
            } else {
                out = 4;
            }

            switch (in.read()) {
            case 0: {
                out = 5 + dummy();
            } break;

            case 1:
                break;

            case 2:
                out = 6;
                break;

            default: if (in.read() > 2)
                out = 7;
            }
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

