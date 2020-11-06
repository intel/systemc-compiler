/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>
#include <stdint.h>
#include <iostream>
#include <string>

// IF with complex condition with &&/||
template<unsigned N>
class A : public sc_module {
public:
    static const bool   CONST_A = 1;
    const sc_uint<3>    CONST_B = 5;
    static const unsigned CONST_C = 5;
    static const unsigned CONST_Z = 0;
    
    SC_HAS_PROCESS(A);

    sc_signal<bool> s{"s"};
    
    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(if_const); sensitive << s;
        SC_METHOD(fcall_const); sensitive << s;
        SC_METHOD(logic_or_const); sensitive << s;
        SC_METHOD(if_logic_or); sensitive << s;
        SC_METHOD(logic_and_const); sensitive << s;
        SC_METHOD(if_logic_and); sensitive << s;
        
        SC_METHOD(complex_logic); sensitive << s;
        SC_METHOD(complex_cond_call); sensitive << s;
        SC_METHOD(if_complex_cond); sensitive << s << ms;
        SC_METHOD(very_complex_cond); sensitive << s << ns;
    }
    
    // Constant in IF condition
    void if_const() {
        int k = 0;
        if (CONST_A) {
            k = 1;
        } else {
            k = 2;
        }
        
        if (CONST_B == CONST_C) {
            k = 1;
        } else {
            k = 2;
        }
        
        if (CONST_Z) {
            k = 2;
        } else {
            k = 1;
        }
        
        if (CONST_B) {
            k = 3;
        }
        
        if (42 && s) {
            k = 3;
        }
    }
    
    int f(int i) {
        return (i-1);
    }
    
    int g(const int& i) {
        return (i+1);
    }

    int h(int& i) {
        return i;
    }
    
    // Constant function call in IF condition
    void fcall_const() {
        int k = 0;
        if (f(1)) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }
        
        if (g(0)) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }
        
        k = 0;
        if (h(k)) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }
    }

    // Constant in binary ||
    void logic_or_const() {
        int k = 0; 
        int i;                      
                                    
        bool b1 = CONST_A || f(1);  // 1
        bool b2 = CONST_A || b1;    // 1
        bool b3 = f(1) || b2;       // 1   
        bool b4 = CONST_Z || g(0);  // 1
        bool b5 = CONST_Z || f(1);  // 0
        bool b6 = b5 || !g(1);      // 0
        bool b7 = !b2 || 0 || CONST_Z; // 0
        bool b8 = b7 || CONST_Z || b2; // 1

        sct_assert_const(b1);
        sct_assert_const(b2);
        sct_assert_const(b3);
        sct_assert_const(b4);
        sct_assert_const(!b5);
        sct_assert_const(!b6);
        sct_assert_const(!b7);
        sct_assert_const(b8);
        
    }
    
     void if_logic_or() {
        int k = 0; 
        
        if (CONST_A || f(1)) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }
        
        if (CONST_A || f(2)) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }

        if (CONST_Z || g(0)) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }

        if (CONST_Z || f(1)) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }
      
        if (f(1) || g(-1) || !CONST_A) {     // 0
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }

        if (f(1) || g(-1) || !CONST_Z) {     // 1
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }
    }
    
    // Constant in binary &&
    void logic_and_const() {
        int k = 0; 
        int i;                      
        
        bool b1 = CONST_A && f(2);  // 1
        bool b2 = CONST_A && b1;    // 1
        bool b3 = f(1) && b2;       // 0   
        bool b4 = CONST_Z && g(0);  // 0
        bool b5 = !CONST_Z && f(1); // 0
        bool b6 = b5 && !g(1);      // 0
        bool b7 = !b2 && 1 && CONST_A; // 0
        bool b8 = b2 && CONST_A && !b7; // 1

        sct_assert_const(b1);
        sct_assert_const(b2);
        sct_assert_const(!b3);
        sct_assert_const(!b4);
        sct_assert_const(!b5);
        sct_assert_const(!b6);
        sct_assert_const(!b7);
        sct_assert_const(b8);
        
    }
    
    void if_logic_and() {
        int k = 0; 
        
        if (CONST_A && f(1)) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }
        
        if (CONST_A && f(2)) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }

        if (CONST_Z && g(0)) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }

        if (CONST_Z && f(1)) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }
      
        if (f(1) && g(-1) && CONST_A) {     // 0
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }

        if (f(2) && g(0) && !CONST_Z) {     // 1
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }
    }
    
    // Constant in binary && and ||
    void complex_logic() {
        bool b1 = CONST_Z || f(1) && CONST_A;
        bool b2 = CONST_A && f(2) || CONST_Z;
        bool b3 = g(0) && CONST_Z && (f(2) || !CONST_A);  // 0
        bool b4 = f(1) || CONST_Z || (f(1) && CONST_A);
        bool b5 = !CONST_Z && (f(1) || CONST_A) && g(1);
        
        sct_assert_const(!b1);
        sct_assert_const(b2);
        sct_assert_const(!b3);
        sct_assert_const(!b4);
        sct_assert_const(b5);
    }
    
    // Constant in binary && and || with function call
     void complex_cond_call() {
        int k = 1;
        
        if (CONST_Z && f(2) || h(k)) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }
        
        if (!g(0) && (CONST_Z || f(2))) {
            k = 3;
            sct_assert_const(0);
        } else {
            k = 4;
        }
        
        if (CONST_A || f(2) && g(0)) {
            k = 5;
        } else {
            k = 6;
            sct_assert_const(0);
        }

        if (!f(2) || g(0) || CONST_Z && g(1)) {
            k = 7;
        } else {
            k = 8;
            sct_assert_const(0);
        }
    }
    
    // Constant in binary && and ||
    sc_uint<4> m;
    sc_signal<sc_uint<4>> ms;
    
    void if_complex_cond() {
        int k = 1;
        m = ms.read();
        
        if (CONST_Z && k == 2 || m == 0) {
            k = 1;
        } else {
            k = 2;
        }

        if (m != 1 && (CONST_Z || m > k)) {
            k = 1;
        } else {
            k = 2;
        }
        
        k = 1;
        if (CONST_A || k != 0 && m == 0) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }

        k = 1;
        if (!k && m == 0 || CONST_A && k == 0) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }
    }
    
    sc_uint<4> n;
    sc_signal<sc_uint<4>> ns;
    
    void very_complex_cond() {
        int k = 1;
        n = ns.read();
        
        if (n == 0 && CONST_A && (k == 0 || CONST_Z || f(2)) && k == 0) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }

        if (CONST_A || (k == 1 && (n == 0 || CONST_A && f(2)))) {
            k = 1;
        } else {
            k = 2;
            sct_assert_const(0);
        }
        
        if (s.read() && !n || (k == n && CONST_A || !g(0))) {
            k = 1;
        } else {
            k = 2;
        }
    }

};

class B_top : public sc_module {
public:

    A<3> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

