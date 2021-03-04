/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// SWITCH statement with IF/loops after
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    
    int                 m;
    int                 k;
    int                 n;

    sc_signal<sc_uint<3>>   s;
    sc_signal<sc_uint<3>>   t;

    SC_CTOR(A) 
    {
        SC_METHOD(switch_compound1); sensitive << s << t << a;
        SC_METHOD(switch_compound2); sensitive << s << t << a;
        SC_METHOD(switch_compound3); sensitive << s << t << a;
        
        SC_METHOD(switch_if1); sensitive << s << t << a;
        SC_METHOD(switch_if2); sensitive << s << t << a;
        SC_METHOD(switch_if3); sensitive << s << t << a;

        SC_METHOD(switch_if_for1); sensitive << s << t << a;
        SC_METHOD(switch_if_for2); sensitive << s << t << a;
        SC_METHOD(switch_if_for3); sensitive << s << t << a;
        SC_METHOD(switch_if_for4); sensitive << s << t << a;
    }
    
    const bool ONE = 1;
    const bool ZERO = 0;
    
    
    void switch_compound1() 
    {
        int i = 0;
        switch (t.read()) {
            case 1 : {
                        {
                            i = 1;
                        }
                        {
                            break;
                        }
                     }
            default: i = 2; break;
        }
    }
    
    void switch_compound2() 
    {
        int i = 0;
        switch (t.read()) {
            case 1 : {
                        {
                            i = 1;
                        }
                        break;
                     }
            case 2 : {
                        {
                            i = 2;
                            break;
                        }
                     }
        }
    }
    
    void switch_compound3() 
    {
        int i = 0;
        switch (t.read()) {
            case 1 : 
            case 2 : {
                        i = 2;
                        break;
                     }
            default: break;
        }
    }
    
// ===========================================================================
    
    // SWITCH with IF after
    void switch_if1() {
        int i;
        switch (t.read()) {
            case 1 : i = 1; break;
            case 2 : i = 2; break;
            default: i = 3;
        }
        
        if (s.read()) {
            i = 4;
        } else {
            i = 5;
            if (t.read()) {
                switch (t.read()) {
                    case 1 : i = 1; break;
                    default: ;
                }
            }
        }
        
        if (i == s.read()) {
            i++;
        }
    }
   
    void switch_if2() {
        k = s.read();
        if (t.read()) {
            int i;
            switch (t.read()) {
                case 1 : i = 1; break;
                case 2 : 
                default: i = 3; break;
            }
        }
        
        if (k == s.read()) {
            
        } else {
            switch (t.read()) {
                case 1 : if (k == 1) {k = 4;} break;
                default: k = 5;
            }
        }
    }
    
    void switch_if3() {
        m = s.read()+1;
        int i;
        switch (t.read()) {
            case 1 : i = 1; break;
            case 2 : {
                        if (s.read() < 3) {
                            i = 2;
                        }
                        break;
                     }
        }
        
        if (s.read() == 1) {
            switch (t.read()) {
                case 1 : 
                default: i = 4;
            }
        }
    }
    

    
    // -----------------------------------------------------------------------

    void switch_if_for1() {
        int i = t.read();
        switch (s.read()) {
            case 1 : i = 1; break;
            case 2 : i = 2; break;
            default : for (int j = 0; j < 7; j++) {i++;}
        }
        
        if (s.read() == 1) {
            for (int j = 0; j < 7; j++) {
                if (t.read()) break;
            }
        }
    }
    
    void switch_if_for2() {
        int i = s.read();
        for (int j = 0; j < 70; j++) {
            if (t.read()) break;
        }
        
        switch (s.read()) {
            case 1: i = 1; break;
            case 2: if (a.read()) {i++;
                    } else {
                       i--;
                       if (i == t.read()) {} else {i--;}
                    }
                    break;
        }
        
        for (int j = 0; j < 40; j++) {
            if (t.read()) i++;
        }
    }
    
    void switch_if_for3() 
    {
        int i = (ONE && s.read()) ? 0 : 1;
        
        switch (i) {
            case 1: i = 1; break;
            default: switch (s.read()) {
                     case 1: i = 2; break;
                     case 2: i = 3; break;
                     }
                break;
        }
        
        sc_uint<24> sum = 0;
        for (int j = 0; j < 10; j++) {
            for (int l = 0; l < 10; l++) {
                sum += l+j;
            }
            
            switch (t.read()) {
                case 1:
                case 2: i = 4; break;
            }
        }
    }
    
    void switch_if_for4() 
    {
        int i = 0;
        if ((s.read() || ZERO) && (ONE && t.read())) {
            switch (s.read()) {
                case 1: i = 1; break;
                case 2: i = 2; break;
                default: ;
            }
        } else {
            switch (s.read()) {
                case 1: i = 3; break;
            }
        }
        
        if (ONE || a.read()) {
            unsigned sum = s.read();
            for (int l = 0; l < 10; l++) {
                sum += l;
            }
        }
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

