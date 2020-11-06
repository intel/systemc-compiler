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
    sc_signal<int>      k_k;

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
    sc_signal<int>      _s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;

    sc_signal<int>      r;
    sc_signal<int>      k_k;
    sc_signal<int>      reg;

    sc_signal<int>*     ps;

    bool                arr[3];
    sc_signal<int>      sarr[3];
    sc_signal<sc_uint<3>>* psarr[3];
    
    struct Local {
        bool a;
        sc_uint<4> b;
    };
    
    Local rec;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : B(name)
    {
        ps = new sc_signal<int>("ps");
        for (int i = 0; i < 3; i++) {
            psarr[i] = new sc_signal<sc_uint<3>>("psarr");
        }

        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    // Simple positive test
    SCT_ASSERT(s, SCT_TIME(0), s, clk.pos());

    // Incorrect time negative tests
    SCT_ASSERT(r, (r.read()), s_d, clk.pos());
    int aaa;
    SCT_ASSERT(s_d2, SCT_TIME(aaa), s_d, clk.pos());

    void test_thread() 
    {
        s = 0; s_d = 0; s_d2 = 0; s_d3 = 0;   
        wait();

        while (true) {
            s_d = s; s_d2 = s_d; s_d3 = s_d2;
            s = !s;
            cout << "." << flush;
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

