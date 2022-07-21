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

class B : public sc_module {
public:
    sc_signal<int>      o;
    sc_signal<int>      r;
    sc_signal<int>      _r;

    SC_CTOR(B) 
    {}
};

// Assertions and SVA generation test
class A : public B 
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_signal<int>      s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;

    sc_signal<int>      r;
    sc_signal<int>      _r;
    sc_signal<int>      reg;

    sc_signal<int>*     ps;

    bool                arr[3];
    sc_signal<int>      sarr[3];

    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : B(name)
    {
        ps = new sc_signal<int>("ps");
        
        SC_CTHREAD(sct_assert_time1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_time2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_loop1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_loop2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_loop3, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // Non-constant time
    void sct_assert_time1() 
    {
        SCT_ASSERT_THREAD(s, SCT_TIME(s.read()), s_d, clk.pos());
        wait();

        while (true) {
            wait();
        }
    }

    // Non-constant time
    void sct_assert_time2() 
    {
        int a;
        SCT_ASSERT_THREAD(s, SCT_TIME(a, 1), s_d, clk.pos());
        wait();

        while (true) {
            wait();
        }
    }

    void sct_assert_loop1() 
    {
        wait();

        while (true) {
            SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
            wait();
        }
    }

    void sct_assert_loop2() 
    {
        wait();

        while (true) {
            int sum = 0;
            for (int i = 0; i < 5; i++) {
                sum +=i;
                SCT_ASSERT_THREAD(i, SCT_TIME(1), sum, clk.pos());
            }
            wait();
        }
    }
    
    void sct_assert_loop3() 
    {
        wait();

        int i = 0;
        while (i < 3) {
            SCT_ASSERT_LOOP(s, SCT_TIME(1), s_d, clk.pos(), i);
            i++;
        }
        
        while (true) {
            wait();
        }
    }

};

class Test_top : public sc_module
{
public:
    sc_signal<bool>        rstn{"rstn"};
    sc_clock clk{"clock", 10, SC_NS};

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

