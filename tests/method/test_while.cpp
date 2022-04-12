/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// while general cases
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;
    
    int                 m = 11;
    int                 k = 12;
    int                 n = 13;
    int*                q;

    sc_signal<bool> s{"s"};

    SC_CTOR(A) {
        SC_METHOD(while_stmt_empty); sensitive << s;
        SC_METHOD(while_stmt1); sensitive << s;
        SC_METHOD(while_stmt2); sensitive << s;
        SC_METHOD(while_stmt3); sensitive << s;
        SC_METHOD(while_stmt4); sensitive << s;
        SC_METHOD(while_stmt5); sensitive << s;
        SC_METHOD(while_stmt6); sensitive << s;
        SC_METHOD(while_stmt7); sensitive << s;

        SC_METHOD(while_const); sensitive << s;
        SC_METHOD(while_sc_type); sensitive << a;
    }
    
    // Empty While
    void while_stmt_empty() {
        int i = 0;
        while (i < 2) {
            i++;
        }
        sct_assert_level(0);
    }

    // Simple while
    void while_stmt1() {
        int k = 0; 
        int i = 0;
        while (i < 2) {
            int k = k + 1; 
            i++;
        }
        sct_assert_level(0);
    }
  
    // While with IF inside
    void while_stmt2() {
        int j = 1;
        int i = 0;
        while (i < 3) {
            i++;
            if (m > 0) {
                j = 2;
            } else {
                j = 3;
            }
            sct_assert_level(1);
        }
        sct_assert_level(0);
        j = 4;
    }

    // While with several inputs from outside
    void while_stmt3() {
        int i = 0;
        int j = 1; int k = 0;
        if (s) {
            j = 2;
        } else {
            j = 3;
        }
        while (i < 2) {   // 2 inputs whilem outside
            k++; i++;
        }
        j = 4;
        sct_assert_level(0);
    }    

    // While with several inputs from outside
    void while_stmt4() {
        int i = 0;
        int j = 1; int k = 0;
        if (s) {
            j = 2;
        }
        while (i < 2) {   // 2 inputs whilem outside
            k++;
            i++;
        }
        sct_assert_level(0);
        j = 3;
    }    
    
    // While with inner while
    void while_stmt5() {
        int k = 0;
        int i = 0;
        while (i < 2) {   
            int j = 0; 
            i++;
            while (j < 3) {
                k = k + 1; j++;
                sct_assert_level(2);
            }
        }
        sct_assert_level(0);
    }     
    
    // While in IF branch
    void while_stmt6() {
        int k = 0;
        int i = 0;
        if (s) {
            while (i < 2) {   
                k = k + 1; i++;
            }
        } else {
            k = 3;
        }
        sct_assert_level(0);
    }         

    // While with some index calculations
    void while_stmt7() {
        int k = 0; int n = 0; int mm = 0;
        int i = 0;
        while (i < 3) {   
            k = k + i; 
            i++;
            n = mm++;
        }
    }     
    
// ---------------------------------------------------------------------------

    // While with false and true constant condition
    void while_const() {
        int k = 0;
        while (false) {
            k = k + 1;
        }
        b.write(k+1);
        
        k = 10;
        while (k < 10) {
            k = k + 1;
        }
        b.write(k+2);
    }
    
    void while_sc_type() {
        k = 1; 
        sc_uint<3> i = 0;
        while (i < 2) {
            k = k + 2; 
            i++;
        }
        sct_assert_const(i == 2);
        sct_assert_const(k == 5);
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

