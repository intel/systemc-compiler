/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// for statement with function call in condition and initialization not supported,
// error reported
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<bool>     s{"s"};
    
    int                 m;
    int                 k;
    
    SC_CTOR(A) {
        SC_METHOD(func_call_cond);
        sensitive << a;
        
        SC_METHOD(func_call_init);
        sensitive << a;

        SC_METHOD(func_call_cond2);
        sensitive << a;
        
        SC_METHOD(func_call_cond3);
        sensitive << a;
        SC_METHOD(func_call_cond4);
        sensitive << a;
        
        SC_METHOD(func_call_cond5);
        sensitive << a;
        
        SC_METHOD(func_call_cond6);
        sensitive << a;
        SC_METHOD(func_call_cond7);
        sensitive << a;

        SC_METHOD(func_call_init2);
        sensitive << a;
    }
    
    // For with function call in condition w/o comparison
    bool cond(int i) {
        return (i < 2);
    }
    void func_call_cond() {
        int k = 0;
        int i = 0;
        for (; cond(i); i++) {
            k = k + 1;
        }
    }
    
    void func_call_cond1() {
        int k = 0;
        // Not supported yet as required "i" declaration/initialization 
        // before loop to provide initialized "i" in first call of @cond()
        for (int i = 0; cond(i); i++) {
            k = k + 1;
        }
    }

    // For with function call in condition with comparison
    unsigned cond2() {
        return 2;
    }
    void func_call_cond2() {
        int k = 0;
        for (int i = 0; i < cond2(); i++) {
            k = k + 1;
        }
    }
    
    // Condition fails from the beginning. Will never enter for loop
    int cond3(int k) {

            return k++;
    }
    void func_call_cond3() {
        int k = 0;
        for (; cond3(k)>10;) {
            k = k + 1;
        }
    }

    // Complex condition
    int cond4(int j) {
            return (j+2);
    }
    int k3;
    void func_call_cond4() {
        k3 = 10;
        for (int i = 0; (2*i) < cond4(k3);) {
            k3 = k3 - 1;
            i++;
        }
    }

    // Different type function
    sc_uint<1> cond5(int j) {
            return (j+2);
    }
    int k0;
    void func_call_cond5() {
        k0 = 10;
        for (sc_uint<3> i = 0; (2*i) < cond5(i);) {
            k0 = k0 - 1;
            i++;
        }
    }

    // Complex condition
    sc_bigint<30> cond6(int j) {
            return (j+2);
    }
    int k1;
    void func_call_cond6() {
        k1 = 10;
        for (sc_bigint<30> i = 0; (2 + i) < cond6(k1);) {
            k1 = k1 - 1;
            i++;
        }
    }

    // Complex condition
    sc_biguint<32> cond7(int j) {
            return (j+2);
    }
    int k2;
    void func_call_cond7() {
        k2 = 10;
        for (sc_biguint<32> i = 0; (i+2) < cond7(k2);) {
            k2 = k2 - 1;
            i++;
        }
    }

// ---------------------------------------------------------------------------
    
    int init1() {
        return 3;
    }
    void func_call_init() {
        int k = 0;
        for (int i = init1(); i < 10; i++) {
            k++;
        }
    }
    
    int init2(int val) {
        return (val+1);
    }
    void func_call_init2() {
        int k = 0;
        for (int i = init2(2); i < 10; i++) {
            k++;
        }
    }
};

class B_top : public sc_module 
{
public:

    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
    }
};

int  sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

