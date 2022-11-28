/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local variables with automatic initialization by @INIT_LOCAL_VARS option
class A : public sc_module {
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(A) 
    {
        SC_METHOD(var1);
        sensitive << s;

        SC_METHOD(var1_unused);
        sensitive << s;

        SC_CTHREAD(var2, clk.pos());
        async_reset_signal_is(rstn, 0);

        SC_CTHREAD(var2_unused, clk.pos());
        async_reset_signal_is(rstn, 0);
    }
        
// ----------------------------------------------------------------------------    
    
    
    void var1() {
	bool a;
        int i;
        sc_uint<4> x;
        
        bool aa = true;
        int ii = 42;
        
        int sum = a + i + aa + ii + x;
        
        if (s.read()) {
            bool b;
            int j;
            sum = b + j;
        }
    }
    
    void var1_unused() {
	bool a;
        int i;
        sc_uint<4> x;
        
        bool aa = true;
        int ii = 42;

        if (s.read()) {
            bool b;
            int j;
        }
    }
    
// ----------------------------------------------------------------------------    
    
    void var2() {
	bool b;
        bool bb = true;
        int j;
        int jj;
        int jjj = 42;
        sc_uint<4> y = jjj + jj + 1;
        int sum = 0;
        wait();
        
        while (true) {
            
            bool c;
            unsigned k;
            
            if (s.read()) {
                bool d;
                long n;
                sum = d + n;
            }
            
            bool d;
            long n;
            
            sum = b + c + k;
            
            wait();
            
            sum = sum + d + n;
        }
    }
    
    void var2_unused() {
	bool b;
        int j;
        wait();
        
        while (true) {
            
            bool c;
            unsigned k;
            
            if (s.read()) {
                bool d;
                long n;
            }
            
            wait();
        }
    }
    
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 10, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

