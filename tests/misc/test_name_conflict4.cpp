/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local/member variables name conflicts: local vars in method and registers 
// from thread, mix of processes
class A : public sc_module {
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(A) 
    {
        SC_CTHREAD(reg_var1, clk.pos());
        async_reset_signal_is(rstn, 0);

        SC_METHOD(local_var1); sensitive << s;

        SC_CTHREAD(reg_var2, clk.pos());
        async_reset_signal_is(rstn, 0);

        SC_METHOD(local_var2); sensitive << s;
    }
    
    sc_signal<int>  t1;
    void reg_var1() 
    {
        int i = 0;
        wait();
        
        while (true) {
            
            long sum = i;
            t1 = sum;
            wait();
        }
    }
    
    sc_signal<int>  t2;
    void local_var1() 
    {
        sc_uint<4> x = s.read();
        long sum = x + 1;
        t2 = sum;
    }


    void reg_var2() 
    {
        bool a = 1;
        sc_uint<4> x;
        wait();
        
        while (true) {
            
            long sum = 42 + x;
            wait();
            s = sum;
            x = a ? s.read() : 0;
        }
    }
    
    sc_signal<int>  t4;
    void local_var2() 
    {
	bool a;
        int i;
        sc_uint<4> x;
        
        i = s.read();
        long sum = a ? i : 0;
        t4 = sum;
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

