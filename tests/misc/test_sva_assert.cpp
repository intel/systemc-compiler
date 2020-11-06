/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
#include <cassert>

// Assertions and SVA generation test
class A : public sc_module 
{
public:

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_in<sc_uint<4>>   a{"a"};
    sc_out<sc_uint<4>>  b{"b"};
    sc_signal<int>      s;
    sc_signal<int>      s_reg;
    int                 m;
    int                 k;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        SC_METHOD(assert_test); 
        sensitive << a;

        SC_METHOD(sc_assert_test); 
        sensitive << a;

        SC_METHOD(sct_assert_method); 
        sensitive << dummy;

        SC_CTHREAD(sct_assert_thread, clk);
        async_reset_signal_is(rstn, false);
    }
    
    // Nothing generated for @assert
    void assert_test() 
    {
        int i = 0;
        assert(i == 0);
    }

    // Nothing generated for @sc_assert
    void sc_assert_test() 
    {
        int i = 0;
        sc_assert(i == 0);
    }

    void sct_assert_method() 
    {
        int i = 0;
        sct_assert(i == 0);
        sct_assert(i == 1, "Message in method");
    }
    
    // Simple immediate assertion test
    void sct_assert_thread() 
    {
        sct_assert(s == 0, "Message in reset");
        s_reg = 0; s = 0;
        wait();
        
        while (true) {
            s_reg = s;
            sct_assert(s == 0);

            wait();
        }
    }
    
 
};

class Test_top : public sc_module
{
public:
    sc_signal<sc_uint<4>>  a{"a"};
    sc_signal<sc_uint<4>>  b{"b"};
    sc_signal<bool>        rstn{"rstn"};
    sc_clock clk{"clk", 10, SC_NS};

    A a_mod{"a_mod"};

    SC_CTOR(Test_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.clk(clk);
        SC_CTHREAD(testProc, clk);
        a_mod.rstn(rstn);

    }

    void testProc() {
    	rstn = 1;
    	wait();
    }


};

int sc_main(int argc, char* argv[])
{
    Test_top test_top{"test_top"};
    sc_start(100, SC_NS);
    return 0;
}

