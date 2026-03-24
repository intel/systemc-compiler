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

// Assertions in base module test

template <unsigned N>
class MIF : public sc_module, sc_interface 
{
public:
    const unsigned NN = N;
    
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_signal<bool>     o;
    sc_signal<bool>     r;
    sc_signal<int>      s;
    
    sc_signal<sc_uint<8>> u;
    sc_uint<8>          m;

    SC_CTOR(MIF) 
    {
        SC_CTHREAD(thread1, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void thread1() 
    {
        o = 0;
        wait();
        SCT_ASSERT_THREAD(o, (1), o, clk.pos());
        
        while (true) {
            wait();
        }
    }
    
    SCT_ASSERT(o, SCT_TIME(0), o, clk);
    SCT_ASSERT(r, SCT_TIME(0), r, clk.pos());  
    SCT_ASSERT(s, SCT_TIME(0,1), !s, clk.pos());  
    SCT_ASSERT(m == N, SCT_TIME(N,NN), m == NN, clk.pos());  

};

class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    sc_signal<int>      r;

    MIF<4>              mif{"mif"};
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        mif.clk(clk);
        mif.rstn(rstn);
        
        SC_CTHREAD(thread2, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    SCT_ASSERT(mif.r, SCT_TIME(2), mif.r, clk.pos());  
    SCT_ASSERT(r, SCT_TIME(2), !r, clk.pos());

    void thread2() 
    {
        wait();
        
        SCT_ASSERT_THREAD(r, (3), mif.r, clk.pos());  

        while (true) {
            r = !r;
            wait();
        }
    }
};

class Test_top : public sc_module
{
public:
    sc_clock clk{"clock", 10, SC_NS};
    sc_signal<bool> rstn;

    A a_mod{"a_mod"};

    SC_CTOR(Test_top) {
        a_mod.clk(clk);
        a_mod.rstn(rstn);
        
        SC_CTHREAD(testProc, clk);

    }

    void testProc() {
    	rstn = 0;
        wait();
    	rstn = 1;
    	wait(10);
        
        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    Test_top test_top{"test_top"};
    sc_start();
    return 0;
}

