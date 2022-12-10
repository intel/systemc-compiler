/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

// Empty IF, loops and function calls
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD(no_sens_method);

        SC_METHOD(empty_if_method);
        sensitive << s;

        SC_METHOD(empty_loop_method);
        sensitive << s;
        
        SC_CTHREAD(empty_loop_thread, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(fcall_method);
        sensitive << s;
        
        SC_CTHREAD(fcall_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void no_sens_method() {
        int a = 1;
    }

    void empty_if_method() {
        int a = 0;
        
        if (s.read()) {
        }

        if (s.read()) {
        } else {
        }

        if (s.read()) a = 1;
            
        if (s.read()) {
        } else {
            a = 2;
        }

        if (s.read()) {
            a = 3;
        } else {
        }
    }

    void empty_loop_method() 
    {
        int a = 0;
        
        for (int i = 0; i < 2; ++i) {
        }

        for (int i = 0; i < 3; ++i) {
            if (a) {}
        }

        int j = 0;
        while (j < 3) {
            ++j;
        }
    }
    
    void empty_loop_thread() 
    {
        wait();
        
        while (true) {
            int a = 0;
            while (s.read()) {wait();}
            wait();
        }
    }
    
    void f() {}
    
    void fcall_method() {
        f();
    }

    void fcall_thread() {
        
        f();
        wait();
        
        while (true) {
            f();
            wait();
        }
    }    
};

int sc_main(int argc, char *argv[]) 
{
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

