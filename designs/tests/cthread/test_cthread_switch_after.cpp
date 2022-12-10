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
        SC_CTHREAD(switch_if_multi1, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(switch_if_for0, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if_for1, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if_for2, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if_for3, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if_for4, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(switch_if_for_multi1, clk.pos());
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(switch_if_for_multi2, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    const bool ONE = 1;
    const bool ZERO = 0;
    
    // SWITCH with IF after
   void switch_if1() {
        int i;
        wait();
        
        while (true) {
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
            wait();
        }
    }
   
    void switch_if2() {
        wait();
        
        while (true) {
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
            wait();
        }
    }
    
    void switch_if3() {
        wait();
        
        while (true) {
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
            
            wait();

            if (s.read() == 1) {
                switch (t.read()) {
                    case 1 : 
                    default: i = 4;
                }
            }
        }
    }
    
    // -----------------------------------------------------------------------

    void switch_if_multi1() {
        int i = 0;
        wait();
        
        while (true) {
            switch (t.read()) {
                case 1: i = 1; break;
                case 2: if (s.read() < 3) {
                            i = 2;
                            wait();         // 1
                        }
                        break;
            }

            if (s.read() == 1) {
                switch (t.read()) {
                    case 1: wait();         // 2
                            i = 4; break;
                    default: i = 5; break;
                }
            }
            wait();     // 3
        }
    }
    
    // -----------------------------------------------------------------------

    void switch_if_for0() {
        wait();
        
        while (true) {
            int i = 0;
            switch (s.read()) {
                case 1 : i = 1; break;
                default : for (int j = 0; j < 7; j++) {i++;}
                          break;
            }

            wait();
        }
    }
    
    void switch_if_for1() {
        wait();
        
        while (true) {
            int i = t.read();
            switch (s.read()) {
                case 1 : i = 1; break;
                case 2 : i = 2; break;
                default : for (int j = 0; j < 7; j++) {i++;}
                          break;
            }

            if (s.read() == 1) {
                for (int j = 0; j < 7; j++) {
                    if (t.read()) break;
                }
            }
            wait();
        }
    }
    
    void switch_if_for2() {
        wait();
        
        while (true) {
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
            wait();
        }
    }
    
    void switch_if_for3() 
    {
        wait();
        
        while (true) {
            int i = (ONE && s.read()) ? 0 : 1;

            switch (i) {
                case 1: i = 1; break;
                default: switch (s.read()) {
                         case 1: i = 2; break;
                         case 2: i = 3; break;
                         }
                    break;
            }
            
            wait();

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
    }
    
    void switch_if_for4() 
    {
        int i = 0;
        wait();
        
        while (true) {
            if ((s.read() || ZERO) && (ONE && t.read())) {
                switch (s.read()) {
                    case 1: i = 1; break;
                    case 2: i = 2; break;
                    default: ;
                }
                wait();         // 1
                
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
            wait();             // 2
        }
    }    
    
    // ------------------------------------------------------------------------
    
    void switch_if_for_multi1() {
        wait();
        
        while (true) {
            int i = t.read();
            switch (s.read()) {
                case 1 : i = 1; break;
                case 2 : i = 2; break;
                default : for (int j = 0; j < 7; j++) {
                            i++; wait();                    // 1
                          }
                          break;
            }

            if (s.read() == 1) {
                for (int l = 0; l < 7; l++) {
                    wait();                                 // 2
                    if (t.read()) break;
                }
            }
            
            switch (s.read()) {
                case 3 : i = 3; break;
                case 4 : if (t.read()) i = 4; break;
            }
            
            wait();                                         // 3
        }
    }
    
    void switch_if_for_multi2() {
        sc_uint<4> n = 0;
        wait();
        
        while (true) {
            switch (s.read()) {
                case 1 : n = 1; break;
                case 2 : n = 2; break;
                default : for (int j = 0; j < 7; j++) {
                            n++; wait();                    // 1
                          }
                          break;
            }
            
            while (n) {
                n--;
                wait();                                     // 2
            }

            for (int l = 0; l < 7; l++) {
                c = 0;
                switch (s.read()) {
                    case 1 : c = 1; break;
                    default: ;
                }

                wait();                                     // 3

                if (t.read()) continue;
                
            }
            wait();                                         // 4
        }
    }

    // ------------------------------------------------------------------------
    
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

