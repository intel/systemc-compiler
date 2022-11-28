/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>
#include <cstdint>
#include <iostream>

using namespace sc_core;


// Constant literals, including comparison with narrow variable
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SC_HAS_PROCESS(A);

    sc_signal<sc_uint<3>> s;
    sc_signal<sc_int<3>> t;
    sc_signal<sc_biguint<66>> bs;
    sc_signal<sc_bigint<66>> bt;
    
    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(comp_narrow_var_pos);
        sensitive << s;
        
        SC_METHOD(comp_narrow_var_neg);
        sensitive << t;

        SC_METHOD(comp_narrow_var_pos_neg);
        sensitive << s;

        SC_METHOD(comp_narrow_var_great);
        sensitive << s << t;
        
        SC_METHOD(comp_narrow_var_big_pos);
        sensitive << bs << bt;
    }
    
    void comp_narrow_var_pos() 
    {
        int k = 0;
        sc_uint<3> a = s.read(); // 0..7
        
        if (a == 15) {
            sct_assert_const(0);
        } else {
            k = 1;
        }
        
        if (8 != a) {
            k = 2;
        } else {
            sct_assert_const(0);
        }

        if (a == 7) {
            k = 3;
        } else {
            k = 3;
        }
        
        if (0 != a) {
            k = 4;
        } else {
            k = 4;
        }
    }
    
    void comp_narrow_var_neg() 
    {
        int k = 0; 
        sc_int<3> a = t.read();   // -4..3
        
        if (a == -5) {
            sct_assert_const(0);
        } else {
            k = 1;
        }
        
        if (4 != a) {
            k = 2;
        } else {
            sct_assert_const(0);
        }

        if (-4 == a) {
            k = 3;
        } else {
            k = 3;
        }
        
        if (a != 3) {
            k = 4;
        } else {
            k = 4;
        }
    }
    
    void comp_narrow_var_pos_neg() 
    {
        int k = 0;
        sc_uint<3> a = s.read(); // 0..7
        
        if (a == -1) {
            sct_assert_const(0);
        } else {
            k = 1;
        }
        
        if (-65535 != a) {
            k = 2;
        } else {
            sct_assert_const(0);
        }
    }

    void comp_narrow_var_great() 
    {
        int k = 0;
        sc_uint<3> a = s.read(); // 0..7
        sc_int<3> b  = t.read(); // -4..3
        
        if (a > 8) {
            sct_assert_const(0);
        } else {
            k = 1;
        }
        
        if (a < 22) {
            k = 2;
        } else {
            sct_assert_const(0);
        }
        
        if (-6 >= b) {
            sct_assert_const(0);
        } else {
            k = 3;
        }
        
        if (b <= 4) {
            k = 4;
        } else {
            sct_assert_const(0);
        }
    }

    
    
    void comp_narrow_var_big_pos() 
    {
        int k = 0;
        sc_biguint<65> a = bs.read();
        sc_biguint<66> C = +(sc_biguint<66>(1)) << 65;
        sc_bigint<65> b = bt.read();
        sc_bigint<67> D = -(sc_bigint<67>(1) << 65);
        
        if (a == C) {
            sct_assert_const(0);
        } else {
            k = 1;
        }
        
        if (C != a) {
            k = 2;
        } else {
            sct_assert_const(0);
        }

        if (a == C >> 1) {
            k = 3;
        } else {
            k = 3;
        }
        
        if (b == D) {
            sct_assert_const(0);
        } else {
            k = 4;
        }
        
        if (D != b) {
            k = 4;
        } else {
            sct_assert_const(0);
        }
        
        if (b != D / 4) {
            k = 5;
        } else {
            k = 5;
        }
        
        if (C < a) {
            sct_assert_const(0);
        } else {
            k = 6;
        }
        
        if (a >= C) {
            sct_assert_const(0);
        } else {
            k = 7;
        }
        
        if (b <= D) {
            sct_assert_const(0);
        } else {
            k = 8;
        }

        if (b > D) {
            k = 9;
        } else {
            sct_assert_const(0);
        }
        
        if (b < D / 7) {
            k = 10;
        } else {
            k = 10;
        }
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    
    return 0;
}

