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
class B : public sc_module {
public:
    const unsigned NN = N;
    
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_signal<bool>     o;
    sc_signal<bool>     r;
    sc_signal<int>      s;
    
    sc_signal<sc_uint<8>> u;
    sc_signal<sc_uint<8>>* up;
    
    sc_uint<8>          m;
    sc_uint<16>         mm;
    sc_uint<8>          marr[3];

    SC_CTOR(B) 
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

class A : public B<3> 
{
public:
    sc_signal<int>      r;

    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : B(name)
    {
        up = new sc_signal<sc_uint<8>>("up");
        
        SC_CTHREAD(thread2, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    // Various time parameters
    SCT_ASSERT(r, SCT_TIME(2), r, clk.pos());  
    SCT_ASSERT(s, SCT_TIME(2), !s, clk.pos());
    SCT_ASSERT(mm == 0, SCT_TIME(2,3), mm == NN, clk.pos());

    void thread2() 
    {
        this->s = 0;
        wait();
        
        SCT_ASSERT_THREAD(this->s, (3), !this->s, clk.pos());  

        while (true) {
            this->s = !this->s;
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
        SC_CTHREAD(testProc, clk);
        a_mod.rstn(rstn);

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

