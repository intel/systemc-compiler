/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// continue statement in loops general cases
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    
    sc_signal<int> si;
    
    SC_CTOR(A) 
    {
        SC_METHOD(for_level);
        sensitive << a << b;

        SC_METHOD(while_level);
        sensitive << a << b;

        SC_METHOD(do_while_level);
        sensitive << a << b;

        SC_METHOD(for_continue1);
        sensitive << a;
        SC_METHOD(for_continue4);
        sensitive << a << b;
        SC_METHOD(for_continue5);
        sensitive << a << b << si;
        SC_METHOD(for_continue6);
        sensitive << a << b << si;
        SC_METHOD(for_continue7);
        sensitive << a << b << si;
        
        SC_METHOD(while_continue1);
        sensitive << a << si;
        SC_METHOD(while_continue2);
        sensitive << a;

        SC_METHOD(do_while_continue1);
        sensitive << a << si;
        SC_METHOD(do_while_continue2);
        sensitive << a;
    }
    
    // Check level after loop with @continue
    void for_level()
    {
        for (int i = 0; i < 3; i++) {
            if (i < 2) continue;
        }
        sct_assert_level(0);
    }

    void while_level()
    {
        int i = 0;
        while (i < 3) {
            i++;
            if (i < 2) continue;
        }
        sct_assert_level(0);
    }
    
    void do_while_level()
    {
        int i = 0;                  // B6
        do {
            i++;                    // B4
            if (i < 2) continue;    // B3
        } while (i < 3);            // B2
        
        sct_assert_level(0);
    }
    
// ----------------------------------------------------------------------------
    
    // Conditional continue in for loop
    void for_continue1() {
        int k = 0;
        for (int i = 0; i < 2; i++) {
            if (a.read()) continue;
            k = k + 1;
        }
        k = 2;
    }
    
    // Continue in inner loop
    void for_continue4() {
        int k = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                if (a.read()) continue;
                k = k + 1;
            }
            sct_assert_level(1);
            if (b.read()) continue;
            k = k - 1;
        }
        sct_assert_level(0);
        k = 2;
    }

    // Two continue in one For loop
    void for_continue5() {
        int k = 0; int m = si.read();
        if (m > 0) {
            for (int i = 0; i < 2; i++) {
                if (a.read()) {
                    if (b.read()) continue;
                    k = k - 1;
                    continue;
                }    
                k = k + 1;
            }
        }    
        k = 2;
    }    
    
    // Two sequential continue
    void for_continue6() {
        int k = 0; int m = si.read();
        if (m > 0) {
            for (int i = 0; i < 2; i++) {
                if (a.read()) continue;
                if (b.read()) continue;
                sct_assert_level(2);
                k = k + 1;
            }
            sct_assert_level(1);
        }    
        k = 2;
        sct_assert_level(0);
    }
    
    // Continue and break in For loop
    void for_continue7() {
        int k = 0; int m = si.read();
        if (m > 0) {
            for (int i = 0; i < 2; i++) {
                if (a.read()) {
                    if (b.read()) break;
                    k = k - 1;
                    continue;
                }    
                k = k + 1;
            }
        }    
        k = 2;
    }
    
// ----------------------------------------------------------------------------

    // Continue in WHILE loop
    void while_continue1() {
        int i = 0;
        int k = 0;
        int m = si.read();
        
        if (m > 0) {
            while (i < 2) {
                i++;
                if (a.read()) continue;
            }
        }    
        k = 2;
    }        

    // Two continue in WHILE loop
    void while_continue2() {
        int i = 0;
        int k = 0;
        while (k < 4) {
            while (i < 2) {
                i++;
                if (a.read()) continue;
                k++;
                sct_assert_level(2);
            }
            if (k == 3) continue;
            sct_assert_level(1);
        }    
        sct_assert_level(0);
        k = 2;
    }        

    
// ----------------------------------------------------------------------------

    // Continue in do/while loop
    void do_while_continue1() {
        int i = 0; int k = 0;
        
        do {
            i++;
            if (i == 1 && a.read()) continue;
        } while (i < 2);

        sct_assert_level(0);
        k = 2;
    }        

    // Two continue in do/while loop
    void do_while_continue2() {
        int i = 0;
        int k = 0;
        do {
            k++;
            do {
                i++;
                if (i < 2 && a.read()) continue;
            } while (i < 3);
        } while (k < 4);   
        
        sct_assert_level(0);
        k = 2;
    }    
    
};

class B_top : public sc_module 
{
public:

    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};
    sc_signal<bool>  c{"c"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
    }
};

int  sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
