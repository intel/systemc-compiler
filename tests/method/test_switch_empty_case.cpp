/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// SWITCH statement with empty cases
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

    sc_signal<bool>         dummy;
    sc_signal<sc_uint<3>>   s;
    sc_signal<sc_uint<3>>   t;

    SC_CTOR(A) {
    
        // Fix #123
        SC_METHOD(switch_if_after_empty); sensitive << s;
        SC_METHOD(switch_if_after_empty2); sensitive << s;
        SC_METHOD(switch_for_after_empty); sensitive << s;
        SC_METHOD(switch_for_after_empty2); sensitive << s;

        SC_METHOD(switch_empty_case1); sensitive << s;
        SC_METHOD(switch_empty_case_if1); sensitive << s;
        SC_METHOD(switch_empty_case_if2); sensitive << s << t;
        SC_METHOD(switch_empty_case_if2a); sensitive << s << t;
        SC_METHOD(switch_empty_case_if2b); sensitive << s << t;
        SC_METHOD(switch_empty_case3); sensitive << s;
        SC_METHOD(switch_empty_case4); sensitive << s;
        SC_METHOD(switch_empty_case4a); sensitive << s;
        SC_METHOD(switch_empty_case4b); sensitive << s;
    
        SC_METHOD(switch_empty_fcall1); sensitive << s;
        SC_METHOD(switch_empty_fcall2); sensitive << s << t;

        SC_METHOD(switch_empty_case); sensitive << s;
        SC_METHOD(switch_break_only); sensitive << s;
        
    }
    
    // BUG from real design accelerators, #123
    void switch_if_after_empty() {
        int i = 0; 
        
        int m = s.read();
        switch (m) {                        // B2
            case 1: //i = 1; break;         // @B4
            case 2:                         // @B6
                    if (s.read() == 1) {
                        m = 1;
                    }                       // B5
                    m = 2;                  // B3
                    break;
        }
        
        i = 0;                              // B1
    }
    
     void switch_if_after_empty2() {
        int i = 0; 
        
        int m = s.read();
        switch (m) {
            case 1: 
            case 2: 
                    if (s.read() == 1) {
                        m = 1;
                    }
                    if (s.read() == 2) {
                        m = 2;
                    }
                    break;
        }
    }
    
    void switch_for_after_empty() {
        int i = 0; 
        
        int m = s.read();
        switch (m) {
            case 1:  
            case 2: 
                for (int i = 0; i < 3; ++i) {m += 1;}
                m = 2;
                break;
            default: m = 3;
        }
        
        i = 0;
    }
    
    void switch_for_after_empty2() {
        int i = 0; 
        int m = s.read();
        switch (m) {
            case 0:
            case 2: for (int j = 0; j < 3; ++j) {}
                    break;
        }
        
        i = 0;
    }
   
    // SWITCH with empty case
    void switch_empty_case1() {
        int i = 0; 
        int m = s.read();
        switch (m) {
            case 1: 
            case 2: i = 2; break;            
            default : i = 3; break;     
        }
        
        i = 0;
    }
    
    void switch_empty_case_if1() {
        int i = 0; 
        int m = s.read();
        switch (m) {
            case 0: 
            case 1: 
            case 2: if (s.read()) i = 1; break;
            case 3:                              
            case 4: i = 2; break;            
            default : i = 3; break;     
        }
        
        i = 0;
    }
    
    void switch_empty_case_if2() {
        int i = 0; 
        
        switch (t.read()) {
            case 1: 
            case 2: 
                if (s.read() == 1) {
                    i = 1;
                } else {
                    if (s.read() == 2) i = 2;
                }
                break;
            default : i = 3; break;     
        }
    }
    
    void switch_empty_case_if2a() {
        int i = 0; 
        
        switch (t.read()) {
            case 1: 
            case 2: 
                if (s.read() == 1) {
                    i = 1;
                } else {
                    if (s.read() == 2) i = 2;
                }
                break;
        }
    }
    
     void switch_empty_case_if2b() {
        int i = 0; 
        
        switch (t.read()) {
            case 1: 
            case 2: 
                if (s.read() == 1) {
                    i = 1;
                } else {
                    if (s.read() == 2) i = 2;
                }
                break;
            case 3:  
                if (s.read() == 1) {
                    i = 1;
                }
                if (s.read() == 2) {
                    i = 2;
                }
        }
    }
    
    void switch_empty_case3() {
        int i = 0; 
        int m = s.read();
        switch (m) {
            case 0: 
            case 1: 
            case 2: switch (s.read()) {
                    case 0: i = 0; break;
                    case 1: i = 1; break;
                    }
                    break;
            case 3:                              
            case 4: i = 2; break;            
        }
        
        i = 0;
    }
    
    void switch_empty_case4() {
        int i = 0; 
        int m = s.read();
        switch (m) {
            case 1: 
            case 2: i = 2;
                    break;
            case 3:                              
            case 4: i = 4; break;            
        }
        
        i = 0;
    }
    
    void switch_empty_case4a() {
        int i; 
        int m = s.read();
        switch (m) {
            case 1: 
            case 2: i = 2;
                    break;
            case 3:                              
            case 4: i = 4; break;            
        }
    }

    void switch_empty_case4b() {
        int i; 
        
        switch (m) {
            case 1: 
            case 2: i = 2; break;
            default:;
        }
    }
    
    
    // SWITCH with function call
    int f(int val) {
        return (val+1);
    }

    void switch_empty_fcall1() 
    {
        int i = 0;
        switch (s.read()) {
            case 1 : 
            case 2 : 
                k = f(2); 
                k++;
                break;
            default: break;
        }
        sct_assert_level(0);
    }    
  
    void switch_empty_fcall2() {
        int i; int k;
        switch (s.read()) {
            case 1 : 
            case 2 : k = 2; break;
            default: k = f(t.read());break;
        }
        i = 0;
    }  
    
    void switch_empty_case() 
    {
        switch (s.read()) {
            case 1: break;
        }
        int i = 0;
    }
    
    void switch_break_only() 
    {
        switch (s.read()) {
            break;
        }
        sct_assert_level(0);
        sct_assert_read(s);
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

