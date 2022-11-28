/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// ....
struct A : public sc_module
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    
    sc_signal<bool>         s;
    sc_signal<bool>         t;
    sc_signal<bool>         ta;
    sc_signal<bool>         tb;
    sc_signal<sc_uint<8>>   r;
    sc_signal<sc_uint<8>>   ra;
    sc_signal<sc_uint<8>>   rb;

    SC_CTOR(A) 
    {
        SC_CTHREAD(loc_var_reset, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(loc_var_body, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(sig_thread, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(assert_sig, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(assert_global, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(assert_local, clk.pos());
        async_reset_signal_is(rst, 1);
    }

     void loc_var_reset() {
        bool j1;
        int j2;
        sc_uint<4> j3;
        wait();
        
        while (true) {
            j1 = 0;
            j2 = 1;
            j3 = 2;
            wait();
        }
    }
     
    void loc_var_body() {
        bool j1 = 1;
        int j2 = j1 ? 2 : 3;
        sc_uint<4> j3 = j2 + 1;
        wait();
        
        while (true) {
            wait();
        }
    }
    
    void sig_thread() {
        s = 1;
        t = 0; r = 0;
        ta = 0; ra = 0;
        wait();
        
        while (true) {
            wait();
        }
    }
    
    void assert_sig() 
    {
        tb = 0; rb = 0;
        SCT_ASSERT_THREAD(t, (0), r.read(), clk.pos());
        SCT_ASSERT_THREAD(tb, (0), rb.read(), clk.pos());
        wait();
        SCT_ASSERT_THREAD(ta, (0), ra.read(), clk.pos());

        while (true) {
            wait();
        }
    }
    
    // Assertion with global variables not used/defined
    bool a;
    int b = 11;
    sc_uint<8> c;
    sc_bigint<8> d = 7;
    sc_uint<8> mm;      // not reg
    
    void assert_global() 
    {
        a = 0; c = 1; mm = 2;
        SCT_ASSERT_THREAD(a, (0), c, clk.pos());
        wait();
        SCT_ASSERT_THREAD(b, (0), d == 1, clk.pos());

        while (true) {
            wait();
        }
    }
    
    // Assertion with local variables not used/defined
    void assert_local() 
    {
        bool e;
        int f;
        sc_uint<8> g;
        sc_bigint<8> h;
        sc_uint<8> nn;  // not reg
        
        SCT_ASSERT_THREAD(e, (0), h == 1, clk.pos());
        wait();
        SCT_ASSERT_THREAD(f, (0), g, clk.pos());

        while (true) {
            wait();
        }
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock  clk("clk", sc_time(1, SC_NS));
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();

    return 0;
}
