/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Constant propagation in for/whie/do..while loops
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};

    sc_signal<bool>     c{"c"};
    
    int                 m;
    int                 k;
    int                 n;

    SC_CTOR(A) 
    {
        SC_METHOD(unstable_loop1); sensitive << a;
        SC_METHOD(unstable_loop2); sensitive << a;
        
        SC_METHOD(simple_for1); sensitive << a;
        SC_METHOD(simple_for1a); sensitive << a;
        SC_METHOD(simple_for1b); sensitive << a;
        SC_CTHREAD(simple_for2, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_METHOD(simple_for3); sensitive << a;
     
        SC_METHOD(continue_in_for1); sensitive << a;
        SC_METHOD(continue_in_for2); sensitive << a;

        SC_METHOD(dowhile_loop); sensitive << a;
        
    }
    
    //----------------------------------------------------------------------
    // Stable loop with unknown iteration number
    void unstable_loop1() {
        // Too many iteration to analyze each, so @k should be unknown
        k = 0;
        for (int i = 0; i < 100; i++) {
            if (i == 99) {k = 1;}
        }
        sct_assert_unknown(k);
    }

    void unstable_loop2() {
        // Too many iteration to analyze each, so @b should be unknown
        k = 0; m = 0;
        bool b = false;
        for (int i = 0; i < 100; i++) {
            b = !b;
            if (b) {k = 1;}
        }
        if (b) {m = 1;}
        if (k == 1) {int l = 1;}
        
        sct_assert_const(k == 1);
        sct_assert_unknown(b);
        sct_assert_unknown(m);
    }
    
    //----------------------------------------------------------------------
    // Simple FOR with known iteration number
    void simple_for1() {
        m = 0;
        for (int i = 0; i < 2; i++) {
            m++;
        }
        
        sct_assert_const(m == 2);
    }
    
    // Simple FOR with known but big iteration number
    void simple_for1a() {
        m = 0;
        for (int i = 0; i < 100; i++) {
            m++;
        }
        if (m) {int l = 1;}
        sct_assert_unknown(m);
    }
    
    // Simple FOR with known but big iteration number and unstable state
    void simple_for1b() {
        bool b = 0;
        for (int i = 0; i < 100; i++) {
            b = !b;
        }
        if (b) {int l = 1;}
        sct_assert_unknown(b);
    }
    
    // Simple FOR with unknown iteration number
    void simple_for2() {
        wait();          // 0
        
        while (true) {
            m = 0; 
            k = a.read();
            
            for (int i = 0; i < k; i++) {
                m = 1; 
                wait();  // 1
            }
            if (m) {int l = 1;}
            sct_assert_unknown(m);
            wait();      // 2
        }
    }
    
    void simple_for3() {
        m = 0; n = 0;
        for (int i = 0; i < 2; i++) {
            m++;
            for (int j = 0; j < m; j++) {
                n++;
            }
        }
        
        sct_assert_const(m == 2);
        sct_assert_const(n == 3);
    }

    // -----------------------------------------------------------------------
    // Conditional @continue in for with wait()
    void continue_in_for1()
    {
        m = 0;
        for (int i = 0; i < 2 ; i++) { 
            if (a.read()) {  
                continue;
            }
            m = 1;
        }         
        sct_assert_unknown(m);
    }    
    
    void continue_in_for2()
    {
        m = 0;
        for (int i = 0; i < 2 ; i++) { 
            if (a.read()) {  
                continue;
            } else {
                m = 1;
            }
        }       
        sct_assert_unknown(m);
    }    
    
    //---------------------------------------------------------------------------    
    void dowhile_loop()
    {
        int i = 0;
        do {
            i++;
        } while (i < 3);
        sct_assert_const(i == 3);
    }    
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

