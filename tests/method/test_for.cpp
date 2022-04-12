/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// for statement general cases
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
    
    SC_CTOR(A) {
        SC_METHOD(requestProc); 
        sensitive << a;
        
        SC_METHOD(for_stmt_empty);
        sensitive << a;
        SC_METHOD(for_stmt1);
        sensitive << a;
        SC_METHOD(for_stmt2);
        sensitive << a;
        SC_METHOD(for_stmt3);
        sensitive << a;
        SC_METHOD(for_stmt3a);
        sensitive << a;
        SC_METHOD(for_stmt4);
        sensitive << a;
        SC_METHOD(for_stmt5);
        sensitive << a;
        SC_METHOD(for_stmt6);
        sensitive << a;
        SC_METHOD(for_stmt7);
        sensitive << a;
        SC_METHOD(for_stmt8);
        sensitive << a;
        SC_METHOD(sc_type_for);
        sensitive << a;

        SC_METHOD(for_false); sensitive << a;
        SC_METHOD(for_false_extr1); sensitive << a;
        SC_METHOD(for_false_extr2); sensitive << a;
        
        // Not supported yet
//        SC_METHOD(for_multi_counter);
//        sensitive << a;
//        SC_METHOD(for_const);
//        SC_METHOD(for_const2);
    }
    
    static const unsigned PORT_NUM = 1;
    sc_signal<bool>       core_req[PORT_NUM];   
    
    // BUG in Demux module real design  -- fixed in nextLevelCalc, no binopPred considered
    void requestProc() 
    {
        bool reqReady = 1;
        for (int port = 0; port < PORT_NUM; port++) { // B5
            reqReady = reqReady && a.read();          // B4, B3, B2
        }   // B1

        for (int port = 0; port < PORT_NUM; port++) { 
            reqReady = reqReady || a.read();          
        }
    }
    
    // Empty for removed
    void for_stmt_empty() {
        for (int i = 0; i < 2; i++) {}
        int i = 0;
    }

    // Simple @for with local variable in body
    void for_stmt1() {
        int j = a.read();
        for (int i = 0; i < 2; i++) {
            int l = a.read();
        }
        for (unsigned i = 0; i < 2; i++) {
            int l = a.read();
        }
        int k = 0;
    }

    // Simple @for with ports and local variable in body
    void for_stmt2() {
        int j = a.read();
        for (int i = 0; i < 2; i++) {
            j = j + 1;
            int l = 2*j;
            if (a.read()) {
                int l = 3*j;
                b.write(l);
            }
            sct_assert_level(1);
            b.write(l);
        }
        sct_assert_level(0);
        b.write(j);
    }

    // For with IF inside
    void for_stmt3() {
        int j = 1;
        for (int i = 0; i < 3; i++) {
            if (i > 1) {
                j = 2;
            } else {
                j = 3;
            }
        }
        sct_assert_level(0);
        j = 4;
    }
    
    void for_stmt3a() {
        int j = 1;
        for (int i = 0; i < 5; i++) {
            if (i > 3) {
                j = 2;
            } else {
                j = 3;
                if (i > 2) {
                    j = 4;
                }
            }
        }
        sct_assert_level(0);
        j = 4;
    }

    // For with several inputs from outside
    void for_stmt4() {
        int i = 0;
        int j = 1; int k = 0;
        if (a.read()) {
            j = 2;
        } else {
            j = 3;
        }
        for (; i < 2; i++) {   // 2 inputs form outside
            k++;
            sct_assert_level(1);
        }
        sct_assert_level(0);
    }    

    // For with several inputs from outside
    void for_stmt5() {
        int i = 0;
        int j = 1; int k = 0;
        if (m > 0) {
            j = 2;
        }
        for (; i < 2; i++) {   // 2 inputs form outside
            k++;
        }
        sct_assert_level(0);
        j = 3;
    }    
    
    // For with inner for loops
    void for_stmt6() {
        int k = 0;
        for (int i = 0; i < 2; i++) {   
            for (int j = 0; j < 3; j++) {
                k = k + 1;
                sct_assert_level(2);
            }
        }
        sct_assert_level(0);
    }

    // For with duplicate variable names
    void for_stmt7() {
        int i = 1;
        int k = 0;
        for (int i = 0; i < 2; i++) {   
            for (int j = 0; j < 3; j++) {
                k = k + i;
            }
        }
        sct_assert_level(0);
    }

    // 3D loop
    sc_signal<sc_uint<8>> s2;
    void for_stmt8() {
        s2 = 0;
        for (int i = 0; i < 2; i++) {   
            for (int j = 0; j < 3; j++) {
                if (a.read()) {
                    for (int k = 0; k < 3; k++) {
                        s2 = k+1;
                    }
                }
                sct_assert_level(2);
                for (int k = 0; k < 3; k++) {
                    if (a.read()) {
                        s2 = k+2;
                    }
                }
            }
            sct_assert_level(1);
        }
        sct_assert_level(0);
    }
    
// ----------------------------------------------------------------------------

    void sc_type_for() 
    {
        int a[8];
        for (sc_uint<4> i = 0; i < 8; i++) {
            a[i] = i;
        }
    
    }
    
     // For with false and true constant condition
    sc_signal<int> r0;
    void for_false() {
        int k = 0;
        for (int i = 0; false; i++) {
            k = k + 1;
        }
        r0 = k;
        sct_assert_level(0);
    }

    sc_signal<int> r1;
    void for_false_extr1() {
        int k = 0;
        int i;
        for (i = 10; i < 5; i++) {
            k = k + 1;
        }
        r1 = k + i;
        sct_assert_level(0);
    }
    
    sc_signal<int> r2;
    void for_false_extr2() {
        int k = 0;
        int i = 10;
        for (; i < 5; i++) {
            k = k + 1;
        }
        r2 = k + i;
        sct_assert_level(0);
    }

// ----------------------------------------------------------------------------
    
    // For with multiple variables
    void for_multi_counter() 
    {
        int a[3];
        for (int i = 0, j = 1; i < 2 && j < 3; i++, j++) {
            a[i] = j;
        }    
    }

    // For with false and true constant condition
    void for_const() {
        int k = 0;
        for (int i = 0; false; i++) {
            k = k + 1;
        }
        
        k = 1;
        for (int i = 0; true; i++) {
            k = k + 1;
        }
        sct_assert_level(0);
    }

    // For with false and true constant condition
    void for_const2() {
        int k = 0;
        int i;
        for (i = 0; false; i++) {
            k = k + 1;
        }
        
        k = 1;
        for (i = 0; true; i++) {
            k = k + 1;
        }
        sct_assert_level(0);
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

