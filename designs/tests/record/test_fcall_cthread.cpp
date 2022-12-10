/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Array of record with functions (methods) called in CTHREAD including
// record array at unknown index
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    sc_signal<int> sig;

    SC_CTOR(A) 
    {
        SC_CTHREAD(single_rec_call_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(single_rec_call_comb, clk.pos());
        async_reset_signal_is(rstn, false);

        // #261 TODO: no initialization of B
        SC_CTHREAD(rec_arr_call_reg, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(rec_arr_call_comb, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(rec_arr_call_reset_reg2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_reg2, clk.pos());
        async_reset_signal_is(rstn, false);

        // #261 TODO: no initialization of B
        SC_CTHREAD(rec_arr_call_unknw_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_unknw_reg2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_unknw_reg3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_unknw_loc, clk.pos());
        async_reset_signal_is(rstn, false);
                
        SC_CTHREAD(rec_arr_call_unknw_loc_in_call, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    struct Simple {
        static const int A = 1;
        const int B = 2;
        bool a;
        int b;
        
        void reset() {
            a = A + a; b = B + b;
        }
        
        void setA(bool par) {
            a = par;
        }
        
        void setB(int par) {
            b = par;
        }

        bool getA() {
            return a;
        }

        int getB() {
            return b;
        }
        
        int locVar() {
            int l = 1;
            return (l + 1);
        }

        int locFcall() {
            int k;
            k = locVar();
            return k;
        }
    };
    
    // Function called in single record, @s is register
    void single_rec_call_reg() 
    {
        Simple s;
        wait();
        
        while (true) {
            s.setA(1);
            wait();

            bool c = s.getA();
        }
    }

    // Function called in single record, @s is not register
    void single_rec_call_comb() 
    {
        Simple r;
        wait();
        
        while (true) {
            r.setA(1);
            bool c = r.getA();
            
            wait();
        }
    }

// ----------------------------------------------------------------------------    
    // Function called in record array, @s is register
    void rec_arr_call_reg() 
    {
        Simple t[2];
        t[0].reset();
        wait();
        
        while (true) {
            t[1].setA(1);
            wait();

            t[0].reset();
            bool c = t[1].getA();
        }
    }

    // Function called in record array, @s is not register
    void rec_arr_call_comb() 
    {
        Simple q[2];
        wait();
        
        while (true) {
            q[1].setA(1);
            bool c = q[1].getA();
            wait();
        }
    }
    
    // Function called in record array, @s is register 
    // (no define at declaration for record array)
    void rec_arr_call_reset_reg2() 
    {
        Simple w[2];
        w[1].setA(2);
        wait();
        bool c = w[1].getA();
        
        while (true) {
            wait();
        }
    }

    void rec_arr_call_reg2() 
    {
        wait();
        
        while (true) {
            Simple z[2];
            bool c = z[1].getA();
            wait();
        }
    }

// ----------------------------------------------------------------------------    
    // Function called in record array at unknown index, @s is register
    void rec_arr_call_unknw_reg() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read() + s[1].B;

            s[1].setA(1);
            bool c = s[j].getA();

            wait();
        }
    }
    
    void rec_arr_call_unknw_reg2() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read();

            s[j].setA(1);
            bool c = s[1].getA();

            wait();
        }
    }
    
    void rec_arr_call_unknw_reg3() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read();

            s[j].setA(1);
            bool c = s[j].getA();

            wait();
        }
    }
    
// ------------------------------------------------------------------------
    // Call function with local variable
    void rec_arr_call_unknw_loc() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read();

            s[j].locVar();

            wait();
        }
    }
    
    // Call function whicch call another function with local variable
    void rec_arr_call_unknw_loc_in_call() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read();

            s[j].locFcall();

            wait();
        }
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);

    sc_start();
    return 0;
}

