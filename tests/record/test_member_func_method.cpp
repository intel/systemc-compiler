/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local record array member call tests
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<sc_uint<2>> sig;
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) 
    {
        SC_METHOD(record_meth0);
        sensitive << sig;

        SC_METHOD(record_meth1);
        sensitive << sig;
        
        SC_METHOD(record_meth2);
        sensitive << sig;

        SC_METHOD(record_meth2a);
        sensitive << sig;

        SC_METHOD(record_meth2b);
        sensitive << sig;

        SC_METHOD(record_meth3);
        sensitive << sig;

        SC_METHOD(record_multi_calls);
        sensitive << sig;
    }
    
    struct Simple {
        bool a;

        void setA(bool par) {
            a = par;
        }

        bool getA() {
            return a;
        }
        
        bool localVar(bool par) {
            bool l;
            l = par || a;
            return l;
        }
    };
    
    void record_meth0()
    {
        Simple s[2];
        
        s[1].setA(true);
        sct_assert_const(s[1].a);
        
        sc_uint<2> i = sig;
        s[i].setA(false);
        sct_assert_unknown(s[1].a);
        sct_assert_array_defined(s[0].a);
    }

    
    // Call method for record array with determinable/non-determinable index
    void record_meth1()
    {
        Simple s[2];
    
        s[1].setA(true);
        bool b = s[1].getA();

        sct_assert_const(s[1].a);
        sct_assert_unknown(s[0].a);
        
        sc_uint<2> i = sig;
        s[i].setA(false);
        b = s[i].getA();
        
        sct_assert_unknown(s[1].a);
        sct_assert_read(s[0].a);
        sct_assert_array_defined(s[0].a);
    }

    // Call method for record array in loop
    void record_meth2() 
    {
        Simple s[2][3];

        bool b = true;
        for (int i = 0; i < 2; i++) {
            s[i][1].setA(i);
            b = b && s[i][1].getA();   // Special case, no function call here
        }
    }

    void record_meth2a() 
    {
        Simple s[2];

        bool b = false;
        for (int i = 0; i < 2; i++) {
            s[i].setA(i);
            b = b || s[i].getA();
        }
    }
    
    // No @getA() call here as b is already true
    void record_meth2b() 
    {
        Simple s[2];

        bool b = true;
        for (int i = 0; i < 2; i++) {
            s[i].setA(i);
            b = b || s[i].getA();
        }
    }
    
    // Call method with local variable
    void record_meth3() 
    {
        Simple s[2];

        bool b = s[1].localVar(1);
        sct_assert_unknown(s[1].a);
        sct_assert_const(b);
        
        sc_uint<2> i = sig;
        b = s[i].localVar(0);
        sct_assert_unknown(b);

        sct_assert_read(s[0].a);
        sct_assert_register(s[0].a);
    }
    
    int f(int par) {
        int l;
        l = par + 1;
        return l;
    }
    
    // Multiple method calls
    void record_multi_calls() 
    {
        Simple s[2];

        sc_uint<2> i = sig;
        bool b = s[i].localVar(true);
        sct_assert_unknown(b);
        
        int j = f(4);
       
        s[i].setA(i);
        
        s[i+1].setA( f(5) );
        sct_assert_unknown(s[1].a);

        f(6);

        sct_assert_array_defined(s[0].a);
    }
};

class B_top : public sc_module {
public:
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
 
