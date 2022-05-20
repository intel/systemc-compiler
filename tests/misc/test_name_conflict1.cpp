/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local/member variables name conflicts: local vars in method and member vars
class A : public sc_module {
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(A) 
    {
        SC_METHOD(local_var); sensitive << s;
        SC_METHOD(member_var); sensitive << s;
    }
        
    
    bool a = true;
    int i;
    sc_uint<4> x = 11;
    long sum;
    
    sc_signal<int>  t1;
    void local_var() 
    {
	bool a;
        int i;
        sc_uint<4> x;
        
        i = s.read();
        long sum = a ? i : x.to_int();
        t1 = sum;
    }

    sc_signal<int>  t2;
    void member_var() 
    {
        i = s.read();
        sum = !a ? i : x.to_int();
        t2 = sum;
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

