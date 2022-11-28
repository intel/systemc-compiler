/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Condition in terminators with type cast
class A : public sc_module 
{
public:
    sc_signal<bool>         a;
    sc_signal<sc_uint<4>>   s;
    sc_signal<sc_int<8>>    t;

    SC_CTOR(A) 
    {
        SC_METHOD(if_var1); sensitive << s << t;
        SC_METHOD(if_const1); sensitive << s << t;
        
        SC_METHOD(switch_var1); sensitive << s << t;
        SC_METHOD(switch_var2); sensitive << s << t;

        SC_METHOD(for_var1); sensitive << s << t;
        SC_METHOD(while_var1); sensitive << s << t;
    }
 
    sc_biguint<66> x;
    sc_bigint<80>  y;
    void if_var1() 
    {
        int k = 0;
        x = s.read();
        if (s.read().to_long()) {
            if ((sc_bigint<12>)t.read() == 0) {
                k = 1;
            }
        }
        if (sc_uint<42>(k+1)) {
            if (sc_uint<42>(k) == 1) {
                k = 2;
            }
        }
        if (x > (sc_int<22>)y || (sc_int<11>)x == k)  {
            k = 3;
        }
    }
    
    void if_const1() 
    {
        int k = 0;
        if ((sc_biguint<12>)51 == s.read()) {
            k = 1;
        } else 
        if ((sc_int<4>)51 == s.read()) {
            k = 2;
            if (((unsigned long)s.read()) == 1) {
                k = 3;
            }
        } else 
        if ((sc_int<2>)111) {
            k = 4;
        }
    }
    
    void switch_var1() 
    {
        int k = 0;
        sc_uint<3> x = s.read(); 

        switch ((sc_uint<2>)x) {
            case 1 : k = 1; break;
            case 2 : k = 2; break;
        }
    }    

    void switch_var2() 
    {
        int k = 0;
        sc_uint<3> x = s.read(); 

        switch (x.to_int()) {
            case 1 : k = 1; break;
            case 2 : k = 2; break;
        }
    }    
    
    void for_var1() 
    {
        int k = 0;
        for (;;) {
            if (k == 10) break;
            k++;
        }
        
        for (int i = 10; sc_uint<3>(i); i--) {
            k--;
        }
    }
    
    void while_var1() 
    {
        int k = 0;
        while (s.read().to_uint()) {
            k++;
        }
        
        while (sc_int<11>(k) > 10) {
            k--;
        }
        
        while (sc_uint<33>(k)) {
            k++;
        }
        
        do {
            k = 0;
        } while (sc_uint<4>(s.read()));
    }
};

int sc_main(int argc, char* argv[])
{
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

