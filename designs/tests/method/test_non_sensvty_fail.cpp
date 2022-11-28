/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Method non-sensitive to some of used signals/ports
class A : public sc_module {
public:
    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;
    
    sc_in<bool>     a{"a"};
    sc_out<bool>    b{"b"};
    sc_out<bool>    c{"c"};
    sc_in<int>      d{"d"};

    sc_signal<bool>  s1{"s1"};
    sc_signal<int>   s2{"s2"};

    SC_CTOR(A) {
        /*SC_METHOD(non_sensitive1);
        sensitive << a;

        SC_METHOD(non_sensitive2);
        sensitive << a;
        
        SC_METHOD(non_sensitive_mult);
        sensitive << s2;
        
        SC_METHOD(non_sensitive_cond);
        sensitive << s2;
        
        SC_METHOD(non_sensitive_dead1);
        sensitive << s1;
        
        SC_METHOD(non_sensitive_dead2);
        sensitive << s1;
        
        SC_METHOD(non_sensitive_empty);
        
        SC_METHOD(false_non_sensitive1);
        sensitive << s1;*/

        SC_METHOD(false_non_sensitive2);
        sensitive << s1;

        SC_METHOD(multi_non_sensitive);
        sensitive << a;
    }
    

    // Check multiple channels non-sensitive are reported
    void multi_non_sensitive()
    {
        if (s1.read() || a.read()) {
            int i = s2.read();
        }
    }
    
    // False non-sensitive error, see #84
    void false_non_sensitive1() 
    {
        bool tmp;
        tmp = a;    // @a not used as @tmp used in dead code
        if (CONST_Z) {
            c = tmp;
        }
        b = s1;
    }
    
    // False non-sensitive error, see #84
    void false_non_sensitive2() 
    {
        bool tmp;
        tmp = a;    // @a not used as @tmp not used
        b = s1;
    }

    
    // Non sensitive to one of used inputs
    void non_sensitive1() 
    {
        b = a || s1;
    }
    
    void non_sensitive2() 
    {
        bool tmp = s1.read();
        b = a && tmp;
    }
    
    // Multiple non sensitive channels
    void non_sensitive_mult() 
    {
        int i = (a) ? s2 : d+1;
        c = b && i > 2;
    }
    
    // Non sensitive in branches
    void non_sensitive_cond() 
    {
        int tmp = 0;
        if (a) {
            if (b) {
                c = d+1;
            } else {
                c = 0;
                tmp = s1;
            }
        }
        s2 = tmp;
    }
    
    // Non sensitive in dead code
    void non_sensitive_dead1() 
    {
        if (CONST_A) {
            b = s1;
        } else {
            b = s2;
        }
    }
    
    // Non sensitive in dead code
    void non_sensitive_dead2() 
    {
        if (CONST_Z) {
            b = s1;
        } else {
            b = s2;
        }
    }
    
    // Non sensitive for empty sensitivity list method
    void non_sensitive_empty() 
    {
        b = s1;
    }
   
};

class B_top : public sc_module {
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};
    sc_signal<int>  d{"d"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
        a_mod.d(d);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

