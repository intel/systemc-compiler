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


// In-process temporal assertions in loops
class A : public sc_module 
{
public:
    static const unsigned N = 2;
    static const int M = N+1;

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

    sc_signal<bool>     sarr[3];
    sc_signal<bool>     sarr_d[3];

    sc_signal<bool>     sarr2[M][N];
    sc_signal<bool>     sarr2_d[M][N];

    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        SC_CTHREAD(sct_assert_1d, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_2d, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // 1-dimensional loop 
    void sct_assert_1d() 
    {
        for (int i = 0; i < 3; i++) {
            SCT_ASSERT_LOOP(sarr[i], (1), sarr_d[i], clk.pos(), i);
        }

        wait();

        for (int j = 0; j < 2; j++) {
            SCT_ASSERT_LOOP(sarr[j], SCT_TIME(N), sarr[j+1], clk.pos(), j);
            SCT_ASSERT_LOOP(sarr[j], SCT_TIME(M, N+2), sarr_d[1], clk.pos(), j);
        }
        
        while (true) {
            wait();
        }
    }
    
    // 2-dimensional loop 
    void sct_assert_2d() 
    {
        a = false;
        for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            SCT_ASSERT_LOOP(sarr2[i][j].read(), (0), (sarr2[i][j] ^ 1) != a, clk.pos(), i, j);
            SCT_ASSERT_LOOP(sarr2[i][j] || s, (1), s_d && sarr2_d[i][j], clk.pos(), i, j);
        }}
        
        wait();

        for (int i = 0; i < M-1; i++) {
        for (int j = 0; j < N; j++) {
            SCT_ASSERT_LOOP(sarr2[i+1][j] || sarr2_d[i][j], SCT_TIME(1,2), s, clk.pos(),  i, j);
        }}
        
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

