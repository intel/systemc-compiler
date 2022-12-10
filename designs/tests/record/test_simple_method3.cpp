/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record member (structure/class, non-module) method call tests
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
    
    Simple r;
    Simple s[2];
    
    void record_meth0()
    {
        s[0].setA(false);
        sct_assert_const(!s[0].a);
        
        s[0].setA(true);
        sct_assert_const(s[0].a);

        sc_uint<2> i = sig;
        s[i].setA(false);
        
        sct_assert_unknown(s[0].a);
    }
    
    // Call method for record array with determinable/non-determinable index
    Simple ss[2];
    void record_meth1()
    {
        ss[1].setA(true);
        bool b = ss[1].getA();
        
        sct_assert_const(ss[1].a);
        
        sc_uint<2> i = sig;
        ss[i].setA(true);
        b = ss[i].getA();

        sct_assert_unknown(ss[1].a);
        sct_assert_read(ss[1].a);
        sct_assert_array_defined(ss[0].a);
    }

    // Call method for record array in loop
    Simple rs[2];
    void record_meth2() 
    {
        bool b = false;
        for (int i = 0; i < 2; i++) {
            rs[i].setA(i);
            b = b && rs[i].getA();   // Special case, no function call here
        }

        sct_assert_const(!rs[0].a);
        sct_assert_const(rs[1].a);
        sct_assert_read(rs[0].a);
        sct_assert_array_defined(rs[0].a);
        //sct_assert_register(rs[0].a);
    }

    Simple ts[2];
    void record_meth2a() 
    {
        bool b = false;
        for (int i = 0; i < 2; i++) {
            ts[i].setA(i);
            b = b || ts[i].getA();
        }
    }
    
    // Call method with local variable
    Simple xs[2];
    void record_meth3() 
    {
        bool b = xs[1].localVar(1);
        sct_assert_const(b);
        
        sc_uint<2> i = sig;
        b = xs[i].localVar(2);
        sct_assert_unknown(b);

        sct_assert_read(xs[0].a);
        sct_assert_register(xs[0].a);
    }
    
    int f(int par) {
        int l;
        l = par + 1;
        return l;
    }
    
    // Multiple method calls
    Simple ys[2];
    void record_multi_calls() 
    {
        sc_uint<2> i = sig;
        bool b = ys[i].localVar(true);
        sct_assert_unknown(b);
        
        int j = f(4);
        
        ys[i].setA(i);
        
        ys[i+1].setA( f(5) );
        sct_assert_unknown(ys[1].a);

        f(6);

        sct_assert_array_defined(ys[0].a);
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
 
