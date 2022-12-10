/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// SWITCH statement with if/loops/break/continue/call in cases
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    
    int                 m = 11;
    int                 k;
    int                 n;

    sc_signal<bool>         dummy;
    sc_signal<sc_uint<3>>   s;
    sc_signal<sc_uint<3>>   t;

    SC_CTOR(A) 
    {
        SC_METHOD(switch_empty1); sensitive << s;
        SC_METHOD(switch_empty2); sensitive << s;
        SC_METHOD(switch_const_default); sensitive << s;
        
        SC_METHOD(switch_stmt1); sensitive << s;
        SC_METHOD(switch_stmt2); sensitive << s;
        SC_METHOD(switch_stmt3); sensitive << s;

        SC_METHOD(switch_if1); sensitive << s;
        SC_METHOD(switch_if2); sensitive << s;
        SC_METHOD(switch_if3); sensitive << s << t;
        SC_METHOD(switch_if3a); sensitive << s << t;
        SC_METHOD(switch_if4); sensitive << s << t;
        SC_METHOD(switch_if4a); sensitive << s << t;
        SC_METHOD(switch_if5); sensitive << s << t;

        SC_METHOD(switch_for1); sensitive << s;
        SC_METHOD(switch_for2); sensitive << s << t;
        SC_METHOD(switch_for3); sensitive << s;
        SC_METHOD(switch_for4); sensitive << s << t;
        
        SC_METHOD(switch_for_empty); sensitive << s << t;
        SC_METHOD(switch_if_empty); sensitive << s << t;
        SC_METHOD(switch_dowhile_empty); sensitive << s << t;
        SC_METHOD(switch_while_empty); sensitive << s << t;
        SC_METHOD(switch_default_while_empty); sensitive << s << t;
        
        SC_METHOD(switch_while1); sensitive << s << t;

        SC_METHOD(switch_break1); sensitive << s;
        SC_METHOD(switch_break2); sensitive << s;
        SC_METHOD(switch_continue1); sensitive << s;
        SC_METHOD(switch_continue2); sensitive << s;
        SC_METHOD(switch_break_continue); sensitive << s;

        SC_METHOD(switch_fcall1); sensitive << s << t;
        SC_METHOD(switch_fcall_empty1); sensitive << s << t;
    }
    
    // SWITCH with default w/o break
    void switch_empty1() {
        int i;
        switch (m) {
            default: i = 1;
        }
        i = 0;
    }

    // SWITCH with default branch break only
    void switch_empty2() {
        int i; 
        int m = s.read();
        switch (m) {
            default : i = 10; break;
        }
        i = 0;
    }

    void switch_const_default() {
        int i;
        const unsigned c = 0;
        
        switch (c) {                   
            case 1: i = 1; break;
            case 2: i = 2; break;   
            default : i = 3;  
        }
        i = 0;
    }
    
    // -----------------------------------------------------------------------
    
    // One SWITCH without default
    void switch_stmt1() {
        int i;
        int m = s.read();
        switch (m) {
            case 1 : i = 1; break;
            case 2 : i = 2; break;
            case 3 : i = 3; break;
        }
        i = 0;
    }
    
    // One SWITCH with default
    void switch_stmt2() {
        int i;
        int m = s.read();
        switch (m) {
            case 1 : i = 1; break;
            case 2 : i = 2; break;
            default: i = 10; 
        }
        i = 0;
    }    
    
    // One SWITCH with break in default
    void switch_stmt3() {
        int i;
        int m = s.read();
        switch (m) {
            case 1 : i = 1; break;
            case 2 : i = 2; break;
            default: i = 10; break;
        }
        i = 0;
    }      
    
    // -----------------------------------------------------------------------
    
    // SWITCH with IF in branches
    void switch_if1() {
        int i; int k;
        int m = s.read();
        switch (m) {
            case 1 : i = 1; break;
            case 2 : 
                    if (m == k) {
                        k = 1;
                    } else {
                        k = 2;
                    }
                    break;
            default: k = 3;
        }
        i = 0;
    }
    
    // SWITCH with IF in default branch
    void switch_if2() {
        int i;
        int m = s.read();
        switch (m) {
            case 1 : i = 1; break;
            case 2 : i = 2; break;
            default: 
                    if (m == k) {
                        k = 3;
                    } else {
                        k = 4;
                    }
                    break;
        }
        i = 0;
    }
    
    // SWITCH with multiple IF
    void switch_if3() {
        int i;
        int m = s.read(); int k;
        switch (m) {
            case 1 : i = 1; break;
            case 2 : 
                    if (t.read() == 0) {
                        k = 3;
                    }
                    if (s.read() == 1) {
                        k = 4;
                    }
                    break;
            default: 
                    if (t.read() == 0) {
                        k = 5;
                        if (s.read() == 1) {
                            k = 6;
                        }
                    }
                    break;
        }
        i = 0;
    }
    
    void switch_if3a() {
        int i; int k;
        switch (s.read()) {
            case 1 : 
                    if (t.read() == 0) {
                        k = 3;
                    } else {
                        if (s.read() == 1) {
                            k = 4;
                        }
                    }
                    break;
            case 2 : 
                    if (s.read() == 1) {
                        k = 5;
                    }
                    break;
        }
        i = 0;
    }
    
    // IF with complex condition
    const bool ONE = 1;
    const bool ZERO = 0;
    
    void switch_if4() {
        int k;
        int i = s.read();
        switch (s.read()) {
            case 1 : 
                    if (i == 1 || t.read()) {
                        k = 3;
                    } else {
                        if (ONE && i == 2) {
                            k = 4;
                        }
                    }
                    break;
            case 2 : 
                    if (ZERO && t.read()) {
                        k = 5;
                    }
                    break;
        }
        i = 0;
    }
    
    void switch_if4a() {
        int k;
        int i = s.read();
        switch (s.read()) {
            case 1 : 
                    if (ONE || i == 1) {
                    } else {
                        k = ZERO && s.read();
                    }
                    break;
            case 2 : 
                    if (t.read() || (ONE || s.read())) {
                        k = ONE || t.read();
                    }
                    break;
        }
        i = 0;
    }
    
    // IF with call in complex
    bool g1(int val) {
        return val == 1;
    }
    
    int g2(int& val) {
        val += 1;
        return val;
    }
    
    void switch_if5() {
        int k;
        int i = s.read();
        switch (s.read()) {
            case 1 : 
                    if (g1(s.read())) {
                        k = 3;
                    }
                    break;
            case 2 : 
                    if (g2(i)) {
                        k = i;
                    }
                    break;
            default: 
                    if (g1(g2(i))) {
                        k = 5;
                    }
                    break;
        }
        i = 0;
    }
    
    // -----------------------------------------------------------------------

    // SWITCH with FOR in case branch
    void switch_for1() {
        int i;
        int k; int m = s.read();
        switch (m) {
            case 1 : k = 1; break;
            case 2 : 
                for (int i = 0; i < 7; i++) {
                    k = k + 1;
                }
                break;
            default: k = 10;
        }
        i = 0;
    }    
    
    void switch_for2() {
        int i;
        int k; int m = s.read();
        switch (s.read()) {
            case 1 : 
                for (int i = 0; i < 3; i++) {
                    k = k + 1;
                }
                for (int j = 0; j < 4; j++) {
                    k = k - 1;
                }
                break;
            case 2 : 
                for (int l = 0; l < 7; l++) {
                    if (l == s.read()) {
                        l++;
                    }
                    for (int ll = 0; ll < 3; ll++) {
                        if (l == t.read()) {
                            l--;
                        }
                    }
                }
                break;
        }
        i = 0;
    }    
    
    // SWITCH with FOR in default branch
    bool arr[3];
    void switch_for3() {
        int i;
        int k; int m = s.read();
        switch (m) {
            case 1 : k = 1; break;
            case 2 : k = 2; break;
            default:
                for (int i = 0; i < 7; i++) {
                    k = k + 1;
                    for (int j = 0; j < 3; j++) {
                        arr[j] = i + j;
                    }
                }
        }
        i = 0;
    }    
    
    void switch_for4() {
        int i;
        int k; int m = s.read();
        switch (s.read()) {
            case 1 : k = 1; break;
            case 2 : k = 2; break;
            default:
                for (int i = 0; i < 7; i++) {
                    k = k + 1;
                }
                if (t.read() == 1) {
                    for (int j = 0; j < 7; j++) {
                        k = k + 1;
                    }
                    i = 3;
                    while (i != 0) {
                        i--;
                    }
                }
        }
        i = 0;
    }    

     // -----------------------------------------------------------------------
    
    // Empty for loop
    void switch_for_empty() {
        int i = 0;
        switch (s.read()) {
            case 1: for (int j = 0; j < 7; j++) {}
                    break;
            default:
                i = 1; 
                break;
        }
        i = 0;
    }    
    
    // Empty if statement
    void switch_if_empty() {
        int i = 0;
        switch (s.read()) {
            case 1: if (t.read()) {} 
                    break;
            default:
                i = 1; 
                break;
        }
        i = 0;
    }  
    
    // Empty do..while
    void switch_dowhile_empty() {
        int i = 0;
        switch (s.read()) {
            case 1: do {i++;} while (i < 10); 
                    break;
            default:
                i = 1; 
                break;
        }
        i = 0;
    }  
    
    // Empty while loop
    void switch_while_empty() {
        int i = 0;
        switch (s.read()) {
            case 1: while (i < 10) { i++; } 
                    break;
            default:
                i = 1; 
                break;
        }
        i = 0;
    }    
    
    // Empty while in default
    void switch_default_while_empty() 
    {
        int i = 0;
        switch (s.read()) 
        {
            case 1: i = 1; break;
            default: 
                while (i < 10) { i++; } 
                break;
        }
        i = 0;
    } 
    
    // Some while
    bool arr2[10];
    void switch_while1() {
        int i = 0;
        switch (s.read()) {
            case 1: while (i < 5) {i++;} break;
            case 2: while (i < 10) {
                        arr2[i] = 0;
                        i++;
                    } 
                    break;
            default:
                if (t.read() == 1) {
                    i = 3;
                    while (i != 0) {
                        i--;
                    }
                }
                break;
        }
        i = 0;
    }   

    // -----------------------------------------------------------------------

    // SWITCH with FOR and BREAK in case branch
    void switch_break1() {
        int i; int k;
        int m = s.read();
        switch (m) {
            case 1 : k = 1; break;
            case 2 : 
                for (int i = 0; i < 7; i++) {
                    k = k + 1;
                    if (m == k) break;
                }
                break;
            default: k = 10;
        }
        i = 0;
    }        
    
    // SWITCH with WHILE and BREAK in default branch
    void switch_break2() {
        int i;
        int k; int m = s.read();
        switch (m) {
            case 1 : k = 1; break;
            case 2 :  
                for (int i = 0; i < 7; i++) {
                    if (s.read()) break;
                }
                break;
            default: 
                int i = 0;
                while (i < 7) {
                    if (m == k) break;
                    k = k + 1; i++;
                }
        }
        i = 0;
    }    
    
    void switch_continue1() {
        int i;
        int k; int m = s.read();
        switch (m) {
            case 1 : k = 1; break;
            case 2 : 
                for (int i = 0; i < 7; i++) {
                    k = k + 1;
                    if (m == k) continue;
                }
                break;
            default: k = 10;
        }
        i = 0;
    }       
    
    void switch_continue2() {
        int i;
        int k; int m = s.read();
        switch (m) {
            case 1: 
                for (int i = 0; i < 7; i++) {
                    if (s.read()) continue;
                }
                break;
            case 2 : k = 2; break;
            default: 
                int i = 0;
                while (i < 7) {
                    if (m == k) continue;
                    k = k + 1; i++;
                }
        }
        i = 0;
    }    
    
    void switch_break_continue() {
        int i;
        int k; int m = s.read();
        switch (m) {
            case 1: 
                for (int i = 0; i < 7; i++) {
                    if (s.read()) continue;
                    
                    int ii = i;
                    while (ii < 3) {
                        ii++;
                        if (s.read()) break;
                    }
                }
                break;
            default:;
        }
        i = 0;
    }    
    
    // -----------------------------------------------------------------------

    // SWITCH with function call
    int f(int val) {
        return (val+1);
    }
    void switch_fcall1() {
        int i; 
        switch (s.read()) {
            case 1 : i = f(1); break;
            case 2 : i = f(2); break;
            default: 
                int j = 0;
                while (j < 7) {
                    if (f(t.read()) == 1) break;
                    j++;
                }
        }
        i = 0;
    }    
    
    void switch_fcall_empty1() {
        int i; 
        switch (s.read()) {
            case 1 : 
            case 2 : i = f(2); break;
        }
        i = 0;
    }    
};

struct dut : sc_core::sc_module {
    typedef dut SC_CURRENT_USER_MODULE;
    dut(::sc_core::sc_module_name) {}
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

