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

// Test for SC_ENABLE_ASSERTIONS is disable in svc_target for target *_sctool
// Errors reported if it is not disabled
class A : public sc_module 
{
public:

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_in<sc_uint<4>>   a{"a"};
    sc_out<sc_uint<4>>  b{"b"};
    sc_signal<int>      s;

    SC_CTOR(A)
    {
        SC_METHOD(assert_test); sensitive << a;     // Errors reported

        SC_METHOD(assert_in_loop_test); sensitive << a;
        
        SC_METHOD(assert_in_if_test); sensitive << a;
    }
    
    sc_signal<int> t0;
    void assert_test() 
    {
        int i = 0;
        assert(i == 0);
        t0 = i;
    }
    
    sc_signal<int> t1;
    void assert_in_loop_test() 
    {
        t1 = 0;
        for (unsigned i = 0; i < 3; ++i) {
            if (a.read()) {
                sc_assert(a.read() == 1);
                t1 = 1;
            }
        }
    }
    
    sc_signal<int> t2;
    void assert_in_if_test() 
    {
        t2 = 0;
        switch (a.read()) {
            case 0: t2 = 1; break;
            default: t2 = 2;
        }
        
        assert(a.read() == 1);
        
        if (a.read()) {
            t2 = 2;
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

