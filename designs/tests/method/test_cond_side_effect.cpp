/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

using namespace sc_core;

// Complex conditions with side effect -- error reported
class A : public sc_module
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;

    int                 m;
    int                 k;
    int*                q;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) : m(1) 
    {
        SC_METHOD(if_side_effect1); sensitive << a;
        SC_METHOD(if_side_effect2); sensitive << a;
        
        SC_METHOD(binary_side_effect); sensitive << a;
        SC_METHOD(binary_side_effect2); sensitive << a;
        
        SC_METHOD(cond_side_effect); sensitive << b << c;
        
        SC_METHOD(cond_fcall); sensitive << a;
        SC_METHOD(binary_unary_fcall); sensitive << a;
    }

    bool f() {
        return k;
    }
    
    // IF condition with function call with side effect
    // This test has incorrect RTL as function body is inlined
    void if_side_effect1() {
        int i = 0;
        if (a.read() && f()) {  // inlined body and temporary variable used here
            i = 1;
        }
        i = 2;
    }
    
    // Expression with side effect
    void if_side_effect2() {
        int i = 0;
        if (a.read() && m++) {  
            i = m;
        }
        i = 2;
    }
    
    void binary_side_effect() {
        int i = 0;
        bool b = a.read() || i++; 
        sct_assert_const(i == 1);
        
        b = a.read() && i++; 
        sct_assert_const(i == 2);
        
        b = true || i++;
        sct_assert_const(i == 2);
        
        b = false || i++;
        sct_assert_const(i == 3);
        
        b = true && i++;
        sct_assert_const(i == 4);
        
        b = false && i++;
        sct_assert_const(i == 4);
    }
    
    void binary_side_effect2() {
        int i = 0;
        bool b = i++ || a.read(); 
        sct_assert_const(i == 1);
        
        b = i++ && a.read(); 
        sct_assert_const(i == 2);
        
        b = true && i++ || a.read(); 
        sct_assert_const(i == 3);

        b = false && i++ && a.read(); 
        sct_assert_const(i == 3);

        b = a.read() || i++ && false; 
        sct_assert_const(i == 4);
    }

    // Side effect in condition
    void cond_side_effect()
    {
        int i = 3;
        bool res;
        res = (i++) ? b : c;
        sct_assert_const(i == 4);
        
        int j = 1;
        res = (i-- == j--) ? b : c;
        sct_assert_const(i == 3);
        sct_assert_const(j == 0);
    }
    
    int g(int par) {
        return par;
    }
    int h(int par) {
        return (par+1);
    }
    void cond_fcall()
    {
        int i = 3;
        int res;
        res = (g(1)) ? h(1) : h(2);
        res = (a.read()) ? g(0) : h(0);
        if (h(1)) {res = 1;}
        res = 1+h(0);
        if (h(1) || h(2)) {res = h(3);}
    }
    
    void binary_unary_fcall()
    {
        int res;
        res = h(1) + h(2);
        res = -h(0);
        res = (h(1));
        res = unsigned(h(1));
        res = a.read() ? h(1) : h(2);
        res = sc_uint<4>(h(2));
    }
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

public:
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

