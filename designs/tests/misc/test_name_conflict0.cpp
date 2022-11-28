/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local and member variables name conflicts
class A : public sc_module {
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(A) 
    {
        SC_METHOD(local_var); sensitive << s;
        SC_METHOD(member_var); sensitive << s;
        SC_METHOD(both_var1); sensitive << s;
        SC_METHOD(both_var2); sensitive << s;
    }
        
    void local_var() 
    {
	bool a;
        int i;
        sc_uint<4> x;
        
        i = s.read();
        long sum = a ? i : x.to_int();
    }

    
    bool a = 1;
    int i;
    sc_uint<4> x= 11;
    long sum;
    
    void member_var() 
    {
        i = s.read();
        sum = !a ? i : x.to_int();
    }


    int j;
    void both_var1() 
    {
	j = 1;      // Module
        int j;
        j = 2;      // Local
        
        s = j;
    }
    
    int k;
    void both_var2() 
    {
        {
            int k;
            k = 2;   // Local
        }
        
	k = 1;      // Module
        s = k;
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

