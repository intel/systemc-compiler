/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Check CPA in SWITCH statement 
class A : public sc_module 
{
public:
    const unsigned N = 3;
    sc_signal<sc_uint<3>>   s;

    SC_CTOR(A) {
        
        SC_METHOD(switch_const1); sensitive << s;
        SC_METHOD(switch_const2); sensitive << s;
        SC_METHOD(switch_const3); sensitive << s;

        SC_METHOD(switch_non_const); sensitive << s;

        SC_METHOD(switch_const_if1); sensitive << s;
        SC_METHOD(switch_const_if1a); sensitive << s;
        SC_METHOD(switch_const_for1); sensitive << s;
        SC_METHOD(switch_const_for1a); sensitive << s;
        
        SC_METHOD(switch_const_empty1); sensitive << s;
        SC_METHOD(switch_const_empty2); sensitive << s;
        SC_METHOD(switch_const_empty3); sensitive << s;
    }
    
    void switch_const1() 
    {
        int i = 0; int j = 0;
        const unsigned c = 1; 
        
        switch (c) {
            case 1: i = 1; j = 2; break; 
            case 2: i++; j++; break; 
            default : i = j;
        }
        sct_assert_const(i == 1);
        sct_assert_const(j == 2);
    }
    
    void switch_const2() 
    {
        int i = 0; int j = 0;
        const unsigned c = 2; 
        
        switch (c) {
            case 1: i = 1; j = 2; break; 
            case 2: i++; j++; break; 
            default : i = j;
        }
        sct_assert_const(i == 1);
        sct_assert_const(j == 1);
    }
    
    void switch_const3() 
    {
        int i = 0; int j = 0;
        const unsigned c = 3; 
        
        switch (c) {
            case 1: i = 1; j = 2; break; 
            case 2: i++; j++; break; 
            default : i = j;
        }
        sct_assert_const(i == 0);
        sct_assert_const(i == 0);
    }
    
    // Check unknown value after switch with non-constant condition
    void switch_non_const() 
    {
        int i = 0; int j = 0;
        
        switch (s.read()) {
            case 1: i = 1; j = 2; break; 
            case 2: i++; j++; break; 
            default : i = j;
        }
        sct_assert_unknown(i);
        sct_assert_unknown(j);
    }
    
    // IF in constant case switch
    void switch_const_if1() {
        int k = 0;
        switch (N) {
            case 1 : 
                    if (s.read() == 0) {
                        k = 3;
                    }
                    break;
            case 3 : 
                    if (s.read() == 1) {
                        k = 4;
                    }
                    break;
            default : break;
        }
        
        sct_assert_unknown(k);
    }
    
    void switch_const_if1a() {
        int k = 0;
        switch (N) {
            case 1 : 
                    if (s.read() == 0) {
                        k = 3;
                    }
                    break;
            case 3 : 
                    if (k == 0) {
                        k = 4;
                    }
                    break;
            default : break;
        }
        
        sct_assert_const(k == 4);
    }
    
    void switch_const_for1() {
        int k;
        switch (N-1) {
            case 1 : k = 1; break;
            case 2 : 
                for (int i = 0; i < 7; i++) {
                    k = k + 1;
                    if (k == s.read()) break;
                }
                break;
            default: k = 10;
        }
        sct_assert_unknown(k);
    }   
    
     void switch_const_for1a() {
        int k;
        switch (N) {
            case 1 : k = 1; break;
            case 2 : 
                for (int i = 0; i < 7; i++) {
                    k = k + 1;
                }
                break;
            default: k = 10;
        }

        for (int j = 0; j < 7; j++) {
            k = k + 1;
            if (k == s.read()) break;
        }
    }   
    
    
    void switch_const_empty1() 
    {
        int i = 0;
        const unsigned c = 1; 
        
        switch (c) {
            case 1: 
            case 2: i = 2; break; 
            default : i = 3;
        }
        sct_assert_const(i == 2);
        
        i = 0;
        switch (c) {
            case 1: 
            case 2: i = 2; break; 
        }
        sct_assert_unknown(i);
    }

    void switch_const_empty2() 
    {
        int i = 0;
        const unsigned c = 2; 
        
        switch (c) {
            case 1: 
            case 2: i = 2; break; 
            default : i = 3;
        }
        sct_assert_const(i == 2);

        i = 0;
        switch (c) {
            case 1: 
            case 2: i = 2; break; 
        }
        sct_assert_unknown(i);
    }
  
    void switch_const_empty3() 
    {
        int i = 0;
        const unsigned c = 3; 
        
        switch (c) {
            case 1: 
            case 2: i = 2; break; 
        }
        sct_assert_const(i == 0);

        switch (c) {
            case 1: 
            case 2: i = 2; break; 
            default : i = 3;
        }
        sct_assert_const(i == 3);
    }
    
  
};

struct dut : sc_core::sc_module {
    typedef dut SC_CURRENT_USER_MODULE;
    dut(::sc_core::sc_module_name) {}
};

class B_top : public sc_module
{
public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

