/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// WHILE statement in method process body analysis
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;
    
    int                 m = 11;
    int                 k;
    int                 n;
    int*                q;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(do_while1); sensitive << dummy;
        SC_METHOD(do_while2); sensitive << dummy;
        SC_METHOD(do_while3); sensitive << dummy;
        SC_METHOD(do_while4); sensitive << dummy;
        SC_METHOD(do_while5); sensitive << dummy;
        SC_METHOD(do_while6); sensitive << dummy;
        SC_METHOD(do_while_fcall); sensitive << dummy;
    }
    
    // Simple do...while
    void do_while1() {
        int i = 0;          // B4
        
        do {
            i++;            // B2
        } while (i < 2);    // B1, B3    
    }
    
    // do..while with IF inside
    void do_while2() {
        int j = 1;
        int i = 0;
        do {
            i++;
            sct_assert_level(1);
            if (m > 0) {
                j = 2;
            } else {
                j = 3;
            }
        } while (i < 3);
        sct_assert_level(0);
        j = 4;
    }
    
    // While with several inputs from outside
    void do_while3() {
        int i = 0;
        int j = 1; int k = 0; int m = dummy.read();
        if (m > 0) {
            j = 2;
        } else {
            j = 3;
        }
        do {   // 2 inputs while outside
            k++; i++;
        } while (i < 2);

        sct_assert_level(0);
        j = 4;
    }    

    // While with several inputs from outside
    void do_while4() {
        int i = 0;
        int j = 1; int k = 0; int m = dummy.read();
        if (m > 0) {
            j = 2;
        }
        do {   // 2 inputs whilem outside
            k++;
            i++;
        } while (i < 2);

        sct_assert_level(0);
        j = 3;
    }    
    
    // While with inner while
    void do_while5() {
        int k = 0; int m = dummy.read();
        int i = 0;
        do {   
            int j = 0; 
            i++;
            do {
                k = k + 1; j++;
            } while (j < 3);
            sct_assert_level(1);            
        } while (i < 2);
        sct_assert_level(0);
    }     
    
    // While in IF branch
    void do_while6() {
        int k = 0; int m = dummy.read();
        int i = 0;
        if (m > 0) {
            do {   
                k = k + 1; i++;
                sct_assert_level(2);
            } while (i < 2);
            sct_assert_level(1);
        } else {
            k = 3;
        }
    }
    
    // While no iteration
    void do_while_const() {
        int k = 0; 
        
        do {   
            k = 2;
        } while (false);
        
        sct_assert_level(0);
        k = 3;
    }
    
    // While with function call
    int f(int i) {
        return (i+1);
    }
    
    void do_while_fcall() {
        int k = 0;
        do {   
            k = f(k);
        } while (k < 3);
        sct_assert_level(0);
    }
};

class B_top : public sc_module 
{
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

