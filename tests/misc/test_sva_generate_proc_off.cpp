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

    SC_CTOR(B) 
    {}
};

// Assertions and SVA generation test in processes, SCT_ASSERT_OFF
class A : public B 
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    bool                a;
    sc_uint<8>          b;
    unsigned            c;
    sc_uint<16>         d;
    sc_signal<int>      s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;

    sc_signal<int>      r;

    sc_signal<int>*     ps;

    bool                arr[3];
    sc_signal<int>      sarr[3];
    sc_signal<sc_uint<3>>* psarr[3];
    
    struct Local {
        bool a;
        sc_uint<4> b;
    };
    
    Local rec;
    
    static const unsigned N = 2;
    static const int M = N+1;

    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : B(name)
    {
        ps = new sc_signal<int>("ps");
        for (int i = 0; i < 3; i++) {
            psarr[i] = new sc_signal<sc_uint<3>>("psarr");
        }

        SC_CTHREAD(sct_assert_imm, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_imm_reg, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_one, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_two_same, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_several, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_several_multi1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_several_multi2, clk.neg());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_several_multi3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread_var1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread_var2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread_loc1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread_loc2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread_loc3, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(sct_assert_cond, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // Immediate asserts
    void sct_assert_imm() 
    {
        int ii = 0; int kk = 1; int mm = 2;
        sct_assert(kk != s);
        wait();
        
        mm = 3;
        sct_assert(mm != s);

        while (true) {
            ii = s;
            wait();
            sct_assert(ii == s_d);
        }
    }

    void sct_assert_imm_reg() 
    {
        int kk = 1; int mm = 2;
        sct_assert(kk != s);
        wait();
        
        mm = 3;
        sct_assert(mm != s);

        while (true) {
            wait();
            sct_assert(mm == kk);
        }
    }

    // One assertion in single wait() thread
    void sct_assert_one() 
    {
        SCT_ASSERT_THREAD(s, 1, s_d, clk.pos());
        wait();

        while (true) {
            wait();
        }
    }
    
    // two similar assertions
    void sct_assert_two_same() 
    {
        SCT_ASSERT_THREAD(s, (1), s_d, clk.pos());
        wait();
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());

        while (true) {
            wait();
        }
    }

    // Several assertions in single wait() thread
    void sct_assert_several()
    {
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d.read() == a, clk.pos());
        SCT_ASSERT_THREAD(b || s, (2,3), s_d2.read() && ((*ps).read() == 1), clk.pos());
        SCT_ASSERT_THREAD(psarr[1]->read(), SCT_TIME(3,0), sarr[0].read() != arr[2], clk.pos());
        wait();

        while (true) {
            wait();
        }
    }
    
    // Several assertions in multi-wait() thread
    void sct_assert_several_multi1()
    {
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        wait();

        while (true) {
            wait();
            
            wait();
        }
    }

    // Negative edge
    void sct_assert_several_multi2()
    {
        SCT_ASSERT_THREAD(s, (1), s_d, clk.pos());
        wait();

        while (true) {
            if (s) {
                wait();
            }
            
            wait();
        }
    }
    
    // Both edges
    void sct_assert_several_multi3()
    {
        int k = 0;
        SCT_ASSERT_THREAD(s, (1), s_d, clk.pos());
        wait();
        SCT_ASSERT_THREAD(s, (2), s_d2, clk.pos());

        while (true) {
            if (s) {
                k = s.read();
                wait();
                
            } else {
                k++;
                wait();
            }
        }
    }
    
    // Member variable in assertions
    void sct_assert_thread_var1() 
    {
        c = s.read();
        SCT_ASSERT_THREAD(c, (1), d == 1, clk.pos());
        wait();

        while (true) {
            d = s_d.read();
            wait();
        }
    }
    
    // Assertion in main loop, no reset checked
    void sct_assert_thread_var2() 
    {
        rec.a = 0;
        wait();

        SCT_ASSERT_THREAD(rec.a, (1), rec.b != s, clk.pos());
        
        while (true) {
            rec.b = s_d.read();
            wait();
        }
    }

    // Local variable in assertions
    void sct_assert_thread_loc1() 
    {
        int i = 0;
        SCT_ASSERT_THREAD(s, (1), i == s_d, clk.pos());
        wait();

        while (true) {
            i = s;
            wait();
        }
    }
    
    // Assertion in main loop, no reset checked
    void sct_assert_thread_loc2() 
    {
        int i = s;
        SCT_ASSERT_THREAD(s, (0), i, clk.pos());
        wait();

        while (true) {
            wait();
        }
    }
    
    void sct_assert_thread_loc3() 
    {
        bool j = false; int i = 0;
        bool k = false; int l = 0;
        SCT_ASSERT_THREAD(i && !j, (1), s_d, clk.pos());
        wait();

        SCT_ASSERT_THREAD(k || l, SCT_TIME(2,3), s, clk.pos());

        while (true) {
            wait();

            j = i == s;
            k = l == s;
            wait();
        }
    }
    
    // Assert in IF with statically evaluated condition
    void sct_assert_cond() 
    {
        if (M == N) SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        wait();
        
        if (N) SCT_ASSERT_THREAD(s, SCT_TIME(2), s_d2, clk.pos());

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

