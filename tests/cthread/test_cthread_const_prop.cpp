/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Constant propagation special cases
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<bool>     c{"c"};
    
    int                 m;
    
    const int* p = nullptr;
    int ci = 42;
    const int* q = &ci;
    
    SC_CTOR(A) 
    {
        SC_CTHREAD(pointer_compare, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(pointer_compare2, clk.pos());
        async_reset_signal_is(nrst, false);
        
        SC_CTHREAD(fifoSyncProc, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(priorityProc, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_METHOD(nested_loops);
        sensitive << a; 
    }

    
    void pointer_compare() {
        wait();
        while (true) {
            const bool b1 = (p == nullptr);
            if (b1) {
                int i = 1;
            }
            sct_assert_const(b1);

            bool b2 = (p == nullptr);
            sct_assert_const(b2);
            
            wait();
        }
    }    
    
    void pointer_compare2() {
        wait();
        while (true) {
            const bool b1 = (q != nullptr);
            if (b1) {
                int i = 1;
            }
            sct_assert_const(b1);

            bool b2 = (q != nullptr);
            sct_assert_const(b2);
            
            wait();
        }
    }    
    
    //----------------------------------------------------------------------
    // Bug in real design module, CPA stop analysis after first iteration
    // as popIndx not changed, so @state is stable -- fixed
    static const unsigned FIFO_LENGTH = 2;
    sc_uint<2> popIndx;
    
    void fifoSyncProc()
    {
        popIndx = 0;
        wait();

        while (true) {
            if (a) {
                if (popIndx == FIFO_LENGTH-1) {
                    popIndx = 0;
                } else {
                    popIndx += 1;   // Produce NO_VALUE here
                }
            }
            wait();
        }
    }
    
    //----------------------------------------------------------------------
    // Bug in real design -- fixed
    sc_signal<sc_uint<2>> rr_first_indx{"rr_first_indx"};
    const unsigned PORT_NUM = 2;
    
    void priorityProc() 
    {
        rr_first_indx = 0;
        wait();

        while (true) {
            sct_assert_level(1);
            if (PORT_NUM > 1) {
                // Normal priority, shift port priorities
                sct_assert_level(2);
                if (true) {
                    sct_assert_level(3);
                    rr_first_indx = (rr_first_indx.read() == 1)
                            ? 2 : ((rr_first_indx.read() == PORT_NUM - 1)
                                   ? 0 : rr_first_indx.read() + 1);
                } else {
                    sct_assert_level(3);
                    rr_first_indx = (rr_first_indx.read() == PORT_NUM - 1)
                                        ? 0 : rr_first_indx.read() + 1;
                }
            }
            sct_assert_level(1);
            wait();
        }
    }
    
    //----------------------------------------------------------------------
    // Check CP performance on nested loops
    void nested_loops() 
    {
        const unsigned SIZE = 5;
        int   arr[SIZE];
    
        // Four nested loops with some unimportant logic
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                for (int k = 0; k < SIZE; k++) {
                    unsigned ll = 0;
                    for (int l = 0; l < SIZE; l++) {
                        ll = l*l;
                    }
                    if (a.read()) {
                        m = k + ll;
                    }
                }
                if (a.read()) {
                    m = j;
                }
            }
            arr[i] = m;
        }
    }
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

