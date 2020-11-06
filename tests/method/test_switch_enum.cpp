/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// SWITCH statement with enum/enum class used as case value 
class A : public sc_module 
{
public:
    sc_signal<unsigned> s;

    SC_CTOR(A) {
        SC_METHOD(switch_enum1); 
        sensitive << s;

        SC_METHOD(switch_enum2); 
        sensitive << s;
    }

    // Simple enumeration
    enum simpleEnum{siFirst = 2, siSecond = 4};
    void switch_enum1() 
    {
        int a = s.read();
        int b;
        
        switch (a) {
            case simpleEnum::siFirst:  
                b = 1; 
                break;
            case simpleEnum::siSecond:  
                b = 2; 
                break;
        }
        
        b = 0;
    }
    
    // Enumeration class
    enum class someEnum{seFirst, seSecond, seThird};
    void switch_enum2() 
    {
        someEnum a = (someEnum)s.read();
        int b;
        
        switch (a) {
            case someEnum::seFirst:  
                b = 1; 
                break;
            case someEnum::seSecond:  
                b = 2; 
                break;
            case someEnum::seThird:  
                b = 3; 
                break;
            default: 
                break;
        }
        b = 0;
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

