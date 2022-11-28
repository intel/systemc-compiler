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
    sc_in<bool>         clk{"clk"};

    sc_signal<bool>      o;
    //sc_signal<int>      r;
    //sc_signal<int>      _r;

    SC_CTOR(B) 
    {
        //SC_CTHREAD(thread0, clk.pos());
    }
    
    void thread0() 
    {
        o = 0;
        wait();
    }
    
    //SCT_ASSERT(o, 0, true, clk);
    
};

// Assertions and SVA generation test
template <unsigned N>
class A : public B 
{
public:
    sc_in<bool>         rstn{"rstn"};

    sc_in<bool>         nrpi;
    sc_out<bool>        nrpo;
    sc_signal<bool>     nrs;
    
    sc_signal<int>      s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;

    sc_signal<sc_uint<8>> u;
    sc_signal<sc_uint<8>>* up;
    
    sc_signal<int>      r;
    sc_signal<int>      _r;
    sc_signal<int>      reg;

    sc_signal<int>*     ps;

    bool                arr[3];
    sc_signal<bool>     sarr[3];
    sc_signal<bool>     sarr_d[3];
    
    struct Local {
        bool a;
        sc_uint<4> b;
    };
    
    Local rec;

    const size_t NN = N;
    const long unsigned M = 1;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name, unsigned m) : B(name)
    {
        auto& val = const_cast<long unsigned&>(M);
        val = m;
        cout << "M " << M << endl;
        
        ps = new sc_signal<int>("ps");
        up = new sc_signal<sc_uint<8>>("up");
        
        SC_CTHREAD(thread1, clk.pos());
        async_reset_signal_is(rstn, false);
        
        //SC_CTHREAD(test_thread, clk.pos());
        //async_reset_signal_is(rstn, false);
        
        //SC_CTHREAD(sct_assert_thread1, clk.pos());
        //async_reset_signal_is(rstn, false);

        //SC_CTHREAD(sct_assert_loop, clk.pos());
        //async_reset_signal_is(rstn, false);
    }
    
    SCT_ASSERT(s, (1,NN), s_d, clk.pos());
    
    //SCT_ASSERT(s, SCT_TIME(2*M), s_d, clk.pos());
    //SCT_ASSERT(s, (M+1), s_d, clk.pos());
    //SCT_ASSERT(s, (M+1,M+2), s_d, clk.pos());
    
    //SCT_ASSERT(s, sct_time(M+1,M+2), s_d, clk.pos());
    //SCT_ASSERT(s, 2*M+1, s_d, clk.pos());
    
    //SCT_ASSERT(o, 2, true, this->clk);
    //SCT_ASSERT(s, 1, s_d, clk.pos());
    //SCT_ASSERT(s, (2,3), s_d2, clk.neg());
    
    void thread1() 
    {
        SCT_ASSERT_THREAD(o, (3), r, clk.pos());
        wait();

        while (true) {
            wait();
        }
    }
    
    void test_thread() 
    {
        nrs = 0;
        s = 0; s_d = 0; s_d2 = 0; s_d3 = 0;
        wait();

        while (true) {
            s_d = s; s_d2 = s_d; s_d3 = s_d2;
            s = !s;
            cout << "." << flush;
            wait();
        }
    }

    // Immediate asserts
    void sct_assert_imm() 
    {
        int i = 0; int k = 1; int m = 2;
        sct_assert(k != s);
        wait();
        
        m = 3;
        sct_assert(m != s);

        while (true) {
            i = s;
            wait();
            sct_assert(i == s_d);
        }
    }
    
    // Simple immediate assertion test
    void sct_assert_thread1() 
    {
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        sct_assert (s == 1, "Imm assert");
        
        int i = 0;
        SCT_ASSERT_THREAD(s, (1), s_d, clk.pos());
        SCT_ASSERT_THREAD(s, (M+1,2), s_d, clk.pos());
        SCT_ASSERT_THREAD(i == 0, (1), i == 1, clk.pos());
        wait();

        SCT_ASSERT_THREAD(s, (M, M+1), s_d, clk.pos());

        while (true) {
            i = 1;
            int j = i;
        
            //sct_assert (i == j, "Imm assert");
        
            wait();
        }
    }
    
    void sct_assert_cond() 
    {
        wait();
        if (true) {
            SCT_ASSERT_THREAD(s, (1), s_d, clk.pos());
        }

        while (true) {
            wait();
        }
    }
    
    void sct_assert_loop() 
    {
        //for (int i = 0; i < 3; i++) {
            //sarr[i] = 0; sarr_d[i] = 0;
        //}
        
        for (int i = 0; i < N+1; i++) {
            SCT_ASSERT_LOOP(sarr[i], (1), sarr_d[i+1], clk.pos(), i);
        }

        wait();

        while (true) {
            wait();
        }
    }

    // Local variable in assertion
    /*void sct_assert_thread2() 
    {
        int ii = 0;
        wait();

        while (true) {
            ii = s;
            SCT_ASSERT_THREAD(s, (1), ii);
            wait();
            
            //ii++;
        }
    }*/
    
};

class Test_top : public sc_module
{
public:
    sc_clock clk{"clock", 10, SC_NS};
    sc_signal<bool> rstn;
    sc_signal<bool> nrp;

    A<1> a_mod{"a_mod", 5};

    SC_CTOR(Test_top) {
        a_mod.clk(clk);
        SC_CTHREAD(testProc, clk);
        a_mod.rstn(rstn);
        a_mod.nrpi(nrp);
        a_mod.nrpo(nrp);

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

