/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// break statement in loops general cases
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    
    sc_signal<int> si;
    
    SC_CTOR(A) 
    {
        SC_METHOD(for_break1);
        sensitive << a;
        SC_METHOD(for_break2);
        sensitive << a;
        SC_METHOD(for_break3);
        sensitive << a << b << si;
        SC_METHOD(for_break4);
        sensitive << a << b << si;
        SC_METHOD(for_break5);
        sensitive << a << b;
        SC_METHOD(for_break6);
        sensitive << a << b;

        SC_METHOD(while_break1);
        sensitive << a;

        SC_METHOD(do_while_break1);
        sensitive << a;
    }
    
    // Conditional break in for loop
    void for_break1() {
        int k = 0;
        for (int i = 0; i < 2; i++) {
            if (a.read()) break;
            k = k + 1;
        }
        sct_assert_level(0);
        k = 2;
    }
    
    // Break from inner loop
    void for_break2() {
        int k = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                if (a.read()) break;
                sct_assert_level(2);
                k = k + 1;
            }
            sct_assert_level(1);
            k = k - 1;
        }
        sct_assert_level(0);
        k = 2;
    }

    // Two breaks
    void for_break3() {
        int k = 0; int m = si.read();
        if (m > 0) {
            for (int i = 0; i < 2; i++) {
                if (a.read()) break;
                if (b.read()) break;
                k = k + 1;
                sct_assert_level(2);
            }
            sct_assert_level(1);
        }    
        sct_assert_level(0);
        k = 2;
    }
    
    // Two breaks
    void for_break4() {
        int k = 0; int m = si.read();
        if (m > 0) {
            for (int i = 0; i < 2; i++) {
                if (a.read()) {
                    if (b.read()) break;
                    k = k - 1;
                    sct_assert_level(3);
                    break;
                }    
                sct_assert_level(2);
                k = k + 1;
            }
            sct_assert_level(1);
        }    
        sct_assert_level(0);
        k = 2;
    }    

    // Break from inner loop
    void for_break5() {
        int k = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 5; j++) {
                sct_assert_level(2);
                if (a.read()) {
                    if (b.read()) break;
                    sct_assert_level(3);
                    k = 1;
                    break;
                }
                k = 2;
            }    
            sct_assert_level(1);
            k = 3;
        }
        sct_assert_level(0);
    }  
    
    // Break from two loops
    void for_break6() {
        int k = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 5; j++) {
                if (b.read()) break;
                k = 2;
            }
            if (b.read()) break;
            sct_assert_level(1);
            k = 3;
        }
        sct_assert_level(0);
    }  
    
// ---------------------------------------------------------------------------

    // Conditional break in while loop
    void while_break1() {
        int i = 10;
        while (i > 0) {
            if (a.read()) break;
            i--;
        }
        sct_assert_level(0);
        
        i = 3;
        while (i > 0) {
            int j = 0;
            while (j < i) {
                if (a.read()) break;
                j++;
            }
            i--;
        }
    }
    
    
// ---------------------------------------------------------------------------

    // Conditional break in do/while loop
    void do_while_break1() {
        int i = 10;
        do {
            if (a.read()) break;
            i--;
        } while (i > 0);
        sct_assert_level(0);
        
        i = 3;
        do {
            int j = 0;
            while (j < i) {
                if (a.read()) break;
                j++;
            }
            i--;
        } while (i > 0);
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
