/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// SWITCH statement inside of if/loops/call
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
        SC_METHOD(switch_if1); sensitive << s << t << a;
        SC_METHOD(switch_if2); sensitive << s << t << a;
        SC_METHOD(switch_if3); sensitive << s << t << a;
        SC_METHOD(switch_if4); sensitive << s << t << a;
        SC_METHOD(switch_if5); sensitive << s << t << a;

        SC_METHOD(switch_if_comp1); sensitive << s << t << a;
        SC_METHOD(switch_if_comp2); sensitive << s << t << a;

        SC_METHOD(switch_for1); sensitive << s << t << a;
        SC_METHOD(switch_for2); sensitive << s << t << a;
        SC_METHOD(switch_for3); sensitive << s << t << a;
        SC_METHOD(switch_for4); sensitive << s << t << a;
        SC_METHOD(switch_for5); sensitive << s << t << a;
        SC_METHOD(switch_for6); sensitive << s << t << a;

        SC_METHOD(switch_while1); sensitive << s << t << a;
        SC_METHOD(switch_while2); sensitive << s << t << a;

        SC_METHOD(switch_call0); sensitive << s << t << a;
        SC_METHOD(switch_call1); sensitive << s << t << a;
        SC_METHOD(switch_call2); sensitive << s << t << a;
    }
    
    const bool ONE = 1;
    const bool ZERO = 0;

    // SWITCH inside of IF
    void switch_if1() {
        int i;
        if (s.read()) {
            switch (t.read()) {
                case 1 : i = 1; break;
                case 2 : i = 2; break;
                default: k = 3;
            }
        }
        i = 0;
    }

    void switch_if2() {
        int i;
        if (s.read()) {
        } else {
            switch (t.read()) {
                case 1 : i = 1; break;
                case 2 : i = 2; break;
                default:;
            }
        }
        i = 0;
    }
    
    void switch_if3() {
        int i;
        int m = s.read();
        if (s.read() || t.read()) {
            switch (m) {
                case 1 : 
                case 2 : i = 2; break;
            }
        }
        i = 0;
    }
    
    void switch_if4() {
        int i;
        if (s.read()) {
            if (t.read()) {
                switch (a.read()) {
                    case 1 : 
                    default : i = 2; break;
                }
            } 
                
            switch (i) {
                case 1 : i = 2; break;
                case 2 : if (a.read()) {}
                         i = 3; break;
            }
        }
            
        switch (s.read()) {
            case 1 : i = 2; break;
        }
    }
            
    void switch_if5() {
        int i;
        if (s.read()) {
            
        } else {
            switch (i) {
                case 1 : i = 2; break;
                case 2 : i = 3; break;
                default: while (i < 3) i++;
            }

            if (t.read()) {
                switch (s.read()) {
                    case 1  : i = 1; 
                              if (s.read()) i--; break;
                    default : i = 2; break;
                }
            }
        }
                         
        if (s.read()) {
            i = 1;
        }
    }
        
        
    bool arr[3];
    void switch_if_comp1() {
        int i;
        if (ZERO || s.read()) {
            switch (i) {
                case 1 : i = 2; break;
                case 2 : i = 3; break;
                default: ;
            }
            
            if (i == 2 || ONE) {
                i++;
            }
        }

        if (ONE && t.read()) {
            switch (i) {
                case 1: for (int i = 0; i < 3; ++i) {
                            arr[i] = a.read();
                        }
                        break;
                case 2 : i = 3; break;
            }
        }
    }    
    
    void switch_if_comp2() {
        int i;
        if (ZERO && s.read()) {
            switch (i) {
                case 1 : i = 2; break;
            }
    
            if (ONE) {
                i++;
            }
        }

        if (ONE || t.read()) {
            switch (i) {
                case 1: if (ZERO || a.read()) {i++;}
                        break;
            }
        }
    }

    // -----------------------------------------------------------------------
    
    // SWITCH inside of FOR
    void switch_for1() {
        int i;
        int l = s.read();
        for (int j = 0; j < 7; j++) 
            switch (l) {
                case 1 : i = 1; break;
                case 2 : i = 2; break;
            }
    }
   
    void switch_for2() {
        int i;
        int m = s.read();
        for (int j = 0; j < 7; j++) {
            switch (s.read()) {
                case 1 : i = 1; break;
                case 2 : i = 2; break;
                default : ;
            }

            switch (m) {
                case 1 : i = 1; break;
                default : if (s.read()) i = 2; break;
            }
        }
    }

    void switch_for3() {
        int i;
        int m = s.read();
        for (int j = 0; j < 3; j++) {
            if (t.read()) {
                switch (s.read()) {
                    case 1 : 
                    case 2 : if (s.read()) i = 1; break;
                    default : {}
                }
            }

            for (int l = 0; l < 3; l++) {
                switch (m) {
                    case 1 : i = 1; break;
                    default : i = 2; break;
                }
            }
        }
    }
    
    void switch_for4() {
        int i; int m = s.read();
        for (int j = 0; j < 3; j++) {
            if (t.read()) {
                switch (s.read()) {
                    case 1 : i = 1; break;
                    default :
                        for (int l = 0; l < 3; l++) {
                            switch (m) {
                                case 1 : i = 1; break;
                                default : i = 2; break;
                            }
                        }
                }
            }
        }
    }
    
    sc_uint<4> arr2d[3][4];
    void switch_for5() {
        int i = 0;
        sc_uint<16> sum = 1;
        for (int j = 0; j < 3; j++) {
            for (int l = 0; l < 4; l++) {
                switch (arr2d[j][l]) {
                    case 1 : i = 1; break;
                    default : i = 2; break;
                }
                sum += i;
            }
        }
    }
    
    void switch_for6() {
        int i;
        for (int j = 1; j < 3; j++) {
            if (j > 0) {
                switch (s.read()) {
                    case 2 : i = 1; break;
                }
            } else {
                i++;
            }
        }
        
        for (int j = 1; j < 3; j++) {
            switch (s.read()) {
                case 3 : i = 1; break;
                default: ;
            }
        }
    }
    
    void switch_while1() {
        int i = 0;
        int j = 0;
        while (j < 4) {
            if (j == s.read()) {
                switch (t.read()) {
                    case 1  : i = 1; break;
                    default : i = 2; break;
                }
            }
            j += 2;
        }
    }

    void switch_while2() {
        int i = 0;
        int j = 5;
        while (j != 0) {
            switch (t.read()) {
                case 1: for (int j = 1; j < 3; j++) {
                            i += j;
                        }
                        break;
                default : i = 2; break;
            }
            j--;
        }
    }
    
    // -----------------------------------------------------------------------

     sc_uint<4> swfunc0(sc_uint<4> val) {
        sc_uint<4> res;
        switch (val) {
            case 1: res = 1; break;
            case 2: res = 2; break;
            default : res = f(3);
        }
        return res;
    }
    
    void switch_call0() 
    {
        int i = swfunc0(1);
    }
    
    sc_uint<4> swfunc1(sc_uint<4> val) {
        switch (val) {
            case 1: return 1;
            case 2: if (val == s.read()) {
                        val = val + 1;
                    }
                    return val;
            default : return (val + 2);
        }
    }
    
    // Some dead cases not eliminates as @liveTerms is not context sensitive
    void switch_call1() {
        int i = 0;
        i = swfunc1(0);
        i = swfunc1(1);
        i = swfunc1(t.read());
    }
    
    
    int f(int val) {
        return (val+1);
    }
    
    sc_uint<4> swfunc2(const sc_uint<4>& val) {
        int l = 0;
        switch (val) {
            case 1: l = 1; break;
            case 2: if (val == s.read()) {
                        l = 2;
                    }
                    break;
            default : l = f(val+1);
                      break;
        }
        return l;
    }

    // Some dead cases not eliminates as @liveTerms is not context sensitive
    void switch_call2() {
        int i = 0;
        i = swfunc2(0);
        i = swfunc2(1);
        i = swfunc2(t.read());
    }
    
// ==========================================================================
// C++ prohibits break not in loop or switch
//    void fb(int& par) {
//        par = 1;
//        break;
//    }
//    
//    void call_in_switch1() {
//        int i = 0;
//        switch (s.read) {
//            case 1: fb(i);
//            default: break;
//        }
//        
//    }
    
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

