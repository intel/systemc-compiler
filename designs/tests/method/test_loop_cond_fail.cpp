/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

using namespace sc_core;

// Loop with condition contains comma/compound statement
class A : public sc_module {
public:
    
    SC_HAS_PROCESS(A);

    sc_in_clk clk;
    sc_signal<bool> nrst;
    
    sc_signal<int> s;

    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(for_cond);
        sensitive << s; 

        SC_METHOD(while_cond);
        sensitive << s; 

        SC_METHOD(do_while_cond);
        sensitive << s; 
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

   
    void for_cond() 
    {
        int m = 0;
        for (int i = 0; i++, i < 3; i++) {
            m += i;
        }
    }
    
    void while_cond() 
    {
        int i = 0;
        while (i++, i < 3) {
            s = i;
        }
    }
    
    void do_while_cond() 
    {
        int i = 3;
        do {
        } while (i--, i != 1);
    }
};


int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"b_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

