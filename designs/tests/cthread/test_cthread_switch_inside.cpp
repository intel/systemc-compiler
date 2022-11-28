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
    sc_in_clk               clk;
    sc_signal<bool>         rstn;

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
        SC_CTHREAD(switch_if1, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if2, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if3, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if4, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if5, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(switch_if_comp1, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if_comp2, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(switch_for1, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_for2, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_for3, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_for4, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_for5, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_for6, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(switch_while1, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_while2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(switch_call1, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_call2, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    const bool ONE = 1;
    const bool ZERO = 0;

    // SWITCH inside of IF
    void switch_if1() {
        int i;
        wait();
        
        while (true) {
            if (s.read()) {
                switch (t.read()) {
                    case 1 : i = 1; break;
                    case 2 : i = 2; break;
                    default: k = 3;
                }
            }
            wait();
        }
    }

    void switch_if2() {
        int i;
        wait();
        
        while (true) {
            if (s.read()) {
            } else {
                switch (t.read()) {
                    case 1 : i = 1; break;
                    case 2 : i = 2; wait(); i++; break;     // 1
                    default:;
                }
            }
            wait();                                         // 2
        }
    }
    
    void switch_if3() {
        int i;
        wait();
        
        while (true) {
            m = s.read();

            wait();

            if (s.read() || t.read()) {
                switch (m) {
                    case 1 : 
                    case 2 : i = 2; break;
                }
            }
        }
    }
    
    void switch_if4() {
        int i;
        wait();
        
        while (true) {
            if (s.read()) {
                if (t.read()) {
                    switch (a.read()) {
                        case 1 : 
                        default : i = 2; break;
                    }
                    wait();                             // 1
                } 

                switch (i) {
                    case 1 : i = 2; break;
                    case 2 : if (a.read()) {}
                             i = 3; break;
                }
            }
            wait();                                     // 2
            

            switch (s.read()) {
                case 1 : i = 2; break;
            }
            wait();                                     // 3
        }
    }
            
    void switch_if5() {
        int i = 0;
        wait();
        
        while (true) {
            if (s.read()) {

            } else {
                switch (i) {
                    case 1 : i = 2; break;
                    case 2 : i = 3; break;
                    default: while (i < 3) {
                                i++; wait();            // 1
                             }
                             break;
                }

                if (t.read()) {
                    switch (s.read()) {
                        case 1  : i = 1; 
                                  wait();               // 2
                                  if (s.read()) i--;
                        default : i = 2; break;
                    }
                }
            }

            wait();                                     // 3
        }
    }
        
    // ------------------------------------------------------------------------
        
    bool arr[3];
    void switch_if_comp1() {
        int i;
        wait();
        
        while (true) {
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
            wait();
        }
    }    
    
    void switch_if_comp2() {
        int i;
        wait();
        
        while (true) {
            if (ZERO && s.read()) {
                switch (i) {
                    case 1 : i = 2; break;
                }

                if (ONE) {
                    i++;
                }
                wait();
            }

            if (ONE || t.read()) {
                switch (i) {
                    case 1: if (ZERO || a.read()) {i++;}
                            break;
                }
            }
            wait();
        }
    }

    // -----------------------------------------------------------------------
    
    // SWITCH inside of FOR
    void switch_for1() {
        int i;
        wait();
        
        while (true) {
            int l = s.read();
            for (int j = 0; j < 7; j++) 
                switch (l) {
                    case 1 : i = 1; break;
                    case 2 : i = 2; break;
                }
            wait();
        }
    }
   
    void switch_for2() {
        int i;
        wait();
        
        while (true) {
            for (int j = 0; j < 7; j++) {
                switch (s.read()) {
                    case 1 : i = 1; break;
                    case 2 : i = 2; break;
                    default : ;
                }

                wait();

                switch (t.read()) {
                    case 1 : i = 1; break;
                    default : if (s.read()) i = 2; break;
                }
            }
        }
    }

    void switch_for3() {
        int i;
        wait();
        
        while (true) {
            for (int j = 0; j < 3; j++) {
                if (t.read()) {
                    switch (s.read()) {
                        case 1 : 
                        case 2 : if (s.read()) i = 1; break;
                        default : {}
                    }
                }

                for (int l = 0; l < 3; l++) {
                    switch (s.read()) {
                        case 1 : i = 1; break;
                        default : i = 2; break;
                    }
                    wait();                         // 1
                }
            }
            wait();                                 // 2
        }
    }
    
    void switch_for4() {
        int i;
        wait();
        
        while (true) {
            for (int j = 0; j < 3; j++) {
                if (t.read()) {
                    switch (s.read()) {
                        case 1 : i = 1; break;
                        default :
                            for (int l = 0; l < 3; l++) {
                                wait();                         // 1
                                switch (t.read()) {
                                    case 1 : i = 1; break;
                                    default : i = 2; break;
                                }
                            } 
                            break;
                    }
                }
                wait();                                         // 2
            }
            wait();                                             // 3
        }
    }
    
    sc_uint<4> arr2d[3][4];
    void switch_for5() {
        int i = 0;
        for (int j = 0; j < 3; j++) {
            for (int l = 0; l < 4; l++) {
                arr2d[j][l] = l / (j+1);
            }
        }
        wait();
        
        while (true) {
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
            wait();
        }
    }
    
    void switch_for6() {
        int i;
        wait();
        
        while (true) {
            for (int j = 1; j < 3; j++) {
                if (j > 0) {
                    switch (s.read()) {
                        case 2 : i = 1; break;
                    }
                } else {
                    i++;
                }
                wait();                             // 1
            }

            for (int j = 1; j < 3; j++) {
                switch (s.read()) {
                    case 3 : i = 1; break;
                    default: ;
                }
            }
            wait();                                 // 2
        }
    }
    
    void switch_while1() {
        int i = 0;
        wait();
        
        while (true) {
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
            wait();
        }
    }

    void switch_while2() {
        int i = 0;
        wait();
        
        while (true) {
            int j = 5;
            while (j != 0) {
                switch (t.read()) {
                    case 1: for (int j = 1; j < 3; j++) {
                                i += j;
                            }
                            break;
                    default : i = 2; break;
                }
                
                wait();                             // 1
                j--;
            }
            wait();                                 // 2    
        }
    }
    
    // -----------------------------------------------------------------------

    sc_uint<4> swfunc1(sc_uint<4> val) {
        switch (val) {
            case 1: return 1;
            case 2: if (val == s.read()) {
                        val = val + 1;
                        wait();
                    }
                    return val;
            default : return (val + 2);
        }
    }
    
    void switch_call1() {
        int i = 0;
        wait();
        
        while (true) {
            i = swfunc1(t.read());
            wait();
        }
    }
    
    
    int f(int par) {
        return (par+1);
    }
    
    sc_uint<4> swfunc2(const sc_uint<3>& val) {
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
        wait();
        return l;
    }

    void switch_call2() {
        int i = 0;
        wait();
        
        while (true) {
            i = swfunc2(t.read());
            wait();
        }
    }
    
    
};

int sc_main(int argc, char* argv[])
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};
    sc_clock clk{"clk", 10, SC_NS};

    A a_mod{"a_mod"};
    a_mod.clk(clk);
    a_mod.a(a);
    a_mod.b(b);
    a_mod.c(c);

    sc_start();
    return 0;
}

