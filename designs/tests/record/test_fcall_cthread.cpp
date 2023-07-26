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

        SC_CTHREAD(rec_arr_call_reg, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(rec_arr_call_comb, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(rec_arr_call_reset_reg2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_reg2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_unknw_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_unknw_reg2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_unknw_reg3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(rec_arr_call_unknw_loc, clk.pos());
        async_reset_signal_is(rstn, false);
                
        SC_METHOD(rec_arr_call_unknw_loc_in_call_m);
        sensitive << sig;
        
        SC_CTHREAD(rec_arr_call_unknw_loc_in_call, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    struct Simple {
        static const int A = 1;
        bool a;
        int b;
        
        void reset() {
            a = A + a; b = A + b;
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
            //k = locVar();
            return k;
        }
    };
    
    // Function called in single record, @s is register
    sc_signal<int> t0;
    void single_rec_call_reg() 
    {
        Simple s;
        wait();
        
        while (true) {
            s.setA(1);
            wait();

            bool c = s.getA();
            t0 = c;
        }
    }

    // Function called in single record, @s is not register
    sc_signal<int> t1;
    void single_rec_call_comb() 
    {
        Simple r;
        wait();
        
        while (true) {
            r.setA(1);
            bool c = r.getA();
            t1 = c;
            
            wait();
        }
    }

// ----------------------------------------------------------------------------    
    // Function called in record array, @s is register
    sc_signal<int> t2;
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
            t2 = c;
        }
    }

    // Function called in record array, @s is not register
    sc_signal<int> t3;
    void rec_arr_call_comb() 
    {
        Simple q[2];
        wait();
        
        while (true) {
            q[1].setA(1);
            bool c = q[1].getA();
            t3 = c;
            wait();
        }
    }
    
    // Function called in record array, @s is register 
    // (no define at declaration for record array)
    sc_signal<int> t4;
    void rec_arr_call_reset_reg2() 
    {
        Simple w[2];
        w[1].setA(2);
        wait();
        bool c = w[1].getA();
        t4 = c;
        
        while (true) {
            wait();
        }
    }

    sc_signal<int> t5;
    void rec_arr_call_reg2() 
    {
        wait();
        
        while (true) {
            Simple z[2];
            bool c = z[1].getA();
            t5 = c;
            wait();
        }
    }

// ----------------------------------------------------------------------------    
    // Function called in record array at unknown index, @s is register
    sc_signal<int> t6;
    void rec_arr_call_unknw_reg() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read() + s[1].A;

            s[1].setA(1);
            bool c = s[j].getA();
            t6 = c;

            wait();
        }
    }
    
    sc_signal<int> t7;
    void rec_arr_call_unknw_reg2() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read();

            s[j].setA(1);
            bool c = s[1].getA();
            t7 = c;

            wait();
        }
    }
    
    sc_signal<int> t8;
    void rec_arr_call_unknw_reg3() 
    {
        Simple s[2];
        wait();
        
        while (true) {
            int j = sig.read();

            s[j].setA(1);
            bool c = s[j].getA();
            t8 = c;

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
    
    void rec_arr_call_unknw_loc_in_call_m() 
    {
        Simple s;
        s.locFcall();
        
        int j = sig.read();
        Simple ss[2];
        ss[j].locFcall();
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

