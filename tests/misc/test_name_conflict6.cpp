/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local variables name conflicts for method empty sensitivity
class A : public sc_module {
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(A) 
    {
        SC_METHOD(empty_proc1);
        SC_METHOD(local_var1); sensitive << s;
        SC_METHOD(empty_proc2);
        SC_METHOD(local_var2); sensitive << s;
    }
        
    void empty_proc1() {
        int i = 1;
        s = i + 1;
    }
    
    void local_var1() 
    {
        int i = 1;
        int j;
        j = 2;
        int sum = i + j;
    }
    
    void empty_proc2() {
        int j = 2;
        s = j + 1;
    }
    
    void local_var2() 
    {
        int j;
        j = 1;
        {
            int j = 2;
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

