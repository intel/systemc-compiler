/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Constant propagation in loops to check iteration number required and stable state 
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};

    sc_signal<bool>     c{"c"};
    sc_signal<int>      s{"s"};
    
    int                 m;
    int                 k;
    int                 n;

    SC_CTOR(A) 
    {
        SC_METHOD(loc_var_loop); sensitive << s;
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_METHOD(unknown_cond_loop); sensitive << s;
        SC_METHOD(unstable_loop1); sensitive << a;
        SC_METHOD(unstable_loop2); sensitive << a;
        SC_METHOD(unstable_loop3); sensitive << a;
        SC_METHOD(unstable_loop3a); sensitive << a;
        SC_METHOD(unstable_loop4); sensitive << a;
        SC_METHOD(unstable_loop4a); sensitive << a;
        SC_METHOD(unstable_loop5); sensitive << a;
        SC_METHOD(unstable_loop6); sensitive << a;
        
        SC_METHOD(simple_for1); sensitive << a;
        SC_METHOD(simple_for1a); sensitive << a;
        SC_METHOD(simple_for1b); sensitive << a;
        SC_CTHREAD(simple_for2, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_METHOD(simple_for3); sensitive << a;
        SC_METHOD(simple_for4); sensitive << a;
     
        SC_METHOD(continue_in_for1); sensitive << a;
        SC_METHOD(continue_in_for2); sensitive << a;
        SC_METHOD(continue_in_for3); sensitive << a;

        SC_METHOD(dowhile_loop); sensitive << a;
    }
    
    void loc_var_loop() {
        unsigned slot = 0;
        for (int i = 0; i < s.read(); i++) {
            slot = (slot == 2) ? 0 : slot + 1;
        }
    }
    
    void threadProc() 
    {
        unsigned slot = 0;
        wait();
        
        while (true) 
        {
            slot = (slot == 2) ? 0 : slot + 1;
            wait();
        } 
    }
    
    // FOR loop with non-evaluated condition
    void unknown_cond_loop() {
        int m = 0;
        for (int i = 0; i < s.read(); i++) {
            m++;
        }
        sct_assert_unknown(m);
    }
    
    //----------------------------------------------------------------------
    // Stable loop with unknown iteration number
    void unstable_loop1() {
        // Too many iteration to analyze each, so @k should be unknown
        int k = 0;
        for (int i = 0; i < 100; i++) {
            if (i == 99) {k = 1;}
        }
        sct_assert_unknown(k);
    }

    void unstable_loop2() {
        // Too many iteration to analyze each, so @b should be unknown
        int k = 0; int m = 0;
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

    // In this example Loop last iteration condition works multiple times to 
    // make state stable with @l1, @l2, @l3 variables become unknown
    void unstable_loop3() {
        bool l1 = false;
        bool l2 = false;
        bool l3 = false;
        for (int i = 0; i < 1026; i++) {
            if (l2) l3 = true;
            if (l1) l2 = true;
            if (i > 20) l1 = true;
        }
        sct_assert_unknown(l1);
        sct_assert_unknown(l2);
        sct_assert_unknown(l3);
    }
    
    void unstable_loop3a() {
        bool first = true;
        unsigned slot = 0;
        for (int i = 0; i < 1026; i++) {
            slot = (slot == 1 && !first) ? 0 : slot + 1;
            first = false;
        }
    }
    
    // Check unstable loop with all iterations analyzed
    void unstable_loop4() {
        bool l1 = false;
        bool l2 = false;
        bool l3 = false;
        for (int i = 0; i < 4; i++) {
            if (l2) l3 = true;
            if (l1) l2 = true;
            if (i == 3) l1 = true;
        }
        sct_assert_const(l1);
        sct_assert_const(!l2);
        sct_assert_const(!l3);
    }
    
    void unstable_loop4a() {
        bool l1 = false;
        bool l2 = false;
        bool l3 = false;
        for (int i = 0; i < 5; i++) {
            if (l2) l3 = true;
            if (l1) l2 = true;
            if (i == 3) l1 = true;
        }
        sct_assert_const(l1);
        sct_assert_const(l2);
        sct_assert_const(!l3);
    }
    
    // Check unstable loop with limited number of iterations equal to maximal 
    // iteration number to check corner cases
    void unstable_loop5() {
        bool l1 = false;
        bool l2 = false;
        bool l3 = false;
        for (int i = 0; i < 10; i++) {
            if (l2) l3 = true;
            if (l1) l2 = true;
            if (i == 9) l1 = true;
        }
        sct_assert_unknown(l1);
        sct_assert_const(!l2);
        sct_assert_const(!l3);
    }
    
    void unstable_loop6() {
        bool l1 = false;
        bool l2 = false;
        bool l3 = false;
        for (int i = 0; i < 11; i++) {
            if (l2) l3 = true;
            if (l1) l2 = true;
            if (i == 10) l1 = true;
        }
        sct_assert_unknown(l1);
        sct_assert_unknown(l2);
        sct_assert_unknown(l3);
    }
    
    //----------------------------------------------------------------------
    // Simple FOR with known iteration number
    void simple_for1() {
        int m = 0;
        for (int i = 0; i < 2; i++) {
            m++;
        }
        
        sct_assert_const(m == 2);
    }
    
    // Simple FOR with known but big iteration number
    void simple_for1a() {
        int m = 0;
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
            int m = 0; 
            int k = a.read();
            
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
        int m = 0; int n = 0;
        for (int i = 0; i < 2; i++) {
            m++;
            for (int j = 0; j < m; j++) {
                n++;
            }
        }
        
        sct_assert_const(m == 2);
        sct_assert_const(n == 3);
    }

    void simple_for4() {
        int m = 0; int n = 0;
        for (int i = 0; i < 2; i++) {
            sct_assert_const(m == i);
            m++;
            for (int j = 0; j < 3; j++) {
                sct_assert_const(n == j+i*3);
                n++;
            }
        }
    }

    // -----------------------------------------------------------------------
    // Conditional @continue in for with wait()
    void continue_in_for1()
    {
        int m = 0;
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
        int m = 0;
        for (int i = 0; i < 2 ; i++) { 
            if (a.read()) {  
                continue;
            } else {
                m = 1;
            }
        }       
        sct_assert_unknown(m);
    }    
    
    void continue_in_for3()
    {
        int m = 0;
        for (int i = 0; i < 5 ; i++) { 
            if (i < 3) continue;
            m++;
        }       
        sct_assert_const(m == 2);
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

