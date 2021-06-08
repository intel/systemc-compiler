/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local variables name conflicts with register next
class A : public sc_module {
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(A) 
    {
        SC_METHOD(empty_proc1);
        SC_METHOD(local_var1); sensitive << s;
        SC_CTHREAD(reg_var1, clk.pos()); async_reset_signal_is(rstn, 0);

        SC_METHOD(empty_proc2);
        SC_METHOD(local_var2); sensitive << s;
        SC_CTHREAD(reg_var2, clk.pos()); async_reset_signal_is(rstn, 0);
    }
        
    void empty_proc1() {
        int i_next = 1;
        s = i_next + 1;
    }
    
    void local_var1() 
    {
        int i_next = 1;
        int sum = i_next + 1;
    }
    
    void reg_var1() 
    {
        int i = 0;
        wait();
        
        while (true) {
            
            long sum = i;
            wait();
        }
    }
    
    void empty_proc2() {
        int i_next = 1;
        s = i_next + 1;
    }
    
    void local_var2() 
    {
        int i_next = 1;
        int sum = i_next + 1;
    }
    
    void reg_var2() 
    {
        int i = 0;
        wait();
        
        while (true) {
            
            long sum = i;
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 10, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

