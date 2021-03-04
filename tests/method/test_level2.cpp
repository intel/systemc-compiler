/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

using namespace sc_core;

// Level determination tests, used to debug level from AST
class A : public sc_module {
public:
    SC_HAS_PROCESS(A);

    sc_in_clk clk;
    sc_signal<bool> nrst;
    sc_signal<int> s;
    sc_signal<int> t;

    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_CTHREAD(local_array, clk.pos());
        async_reset_signal_is(nrst, false);
        
        SC_METHOD(empty_for);
        sensitive << s;
        
        SC_METHOD(empty_for2); sensitive << s;
        SC_METHOD(empty_for2_extr); sensitive << s;
        SC_METHOD(empty_for3_extr); sensitive << s;
        SC_METHOD(empty_for4_extr); sensitive << s;
        
        SC_METHOD(empty_do);
        sensitive << s; 
        
        SC_CTHREAD(for_stmt_wait, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(complex_logic); sensitive << s; 
        
        SC_CTHREAD(simple_for_wait, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(loop_cntr_name); sensitive << s; 
        SC_METHOD(loop_cntr_name2); sensitive << s;
        
        SC_METHOD(multi_for); sensitive << s; 
        SC_METHOD(do_while3); sensitive << s; 
        SC_METHOD(func_with_loop); sensitive << s;
        SC_METHOD(switch_for2); sensitive << s; 
        
        SC_METHOD(switch_if); sensitive << s; 
        SC_METHOD(switch_if_const); sensitive << s; 
        
        SC_CTHREAD(while_wait, clk.pos());
        async_reset_signal_is(nrst, false);
        
        SC_METHOD(two_do_while); sensitive << s; 
                
        SC_CTHREAD(break_in_while_for, clk.pos());
        async_reset_signal_is(nrst, false);
        
        SC_METHOD(empty_loop_method); sensitive << s;
        
        SC_CTHREAD(switch_if3, clk.pos());
        async_reset_signal_is(nrst, false);
        
        SC_CTHREAD(two_loops, clk.pos());
        async_reset_signal_is(nrst, false);
        
        SC_CTHREAD(if_and_loop, clk.pos());
        async_reset_signal_is(nrst, false);
        
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

   
    void local_array() 
    {
        wait();
        
        while (true) {
            bool arr[3];
            
            wait();
        }
    }
    
    void empty_for() 
    {
        for (int i = 0; i < 3; i++) {
            int k = i;
            for (int j = 0; j < 2; j++) {
                int l = i+j;
                sct_assert_const(l == i+j);
            }
        }
        int m = 0;
    }
    
    void empty_for2() 
    {
        int m = 0;
        for (int i = 0; i < 3; i++) {
        }

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; ++j) {
            }
        }
        
        if (s.read()) {
            for (int j = 0; j < 4; --j) {
            }
        }
    }
    
    // External counter -- no remove loop
    void empty_for2_extr() 
    {
        int m = 0;
        int i;
        for (i = 0; i < 3; i++) {}
        for (; i < 6; i++) {}
        for (int j = 0; i < 9; i++) {}
    }
    
    void empty_for3_extr() 
    {
        int m = 0;
        int i = 42;
        for (; i >40; i--) {}
        for (i++; i >38; i--) {}
        for (i = i-1; i > 0; i--) {}
    }
    
    void empty_for4_extr() 
    {
        int m = 1;
        for (int i = 0; i < 3; i += m) {}
    }

    void empty_do() 
    {
        int i = 0;
        do {
        } while (i < 3);
    }
    
    void for_stmt_wait()
    {
        while (true) {
            wait();         // 0
            
            for (int i = 0; i < 2; i++) {   
                wait();     // 1
            }                               
        }
    }
    
    // Constant in binary && and ||
    int f(int i) {
        return (i-1);
    }
    int g(const int& i) {
        return (i+1);
    }
    static const bool   CONST_A = 1;
    static const unsigned CONST_Z = 0;
    
    void complex_logic() {
        bool b3 = CONST_Z && (f(2) || CONST_Z);  
        bool b4 = f(1);
    }
    
    void simple_for_wait() {
        wait();          // 0
        
        while (true) {
            int k = s.read();
            
            for (int i = 0; i < k; i++) {
                wait();  // 1
            }
            wait();      // 2
        }
    }
    
    // ...
    void h(int i) {
        //return (i+1);
    }
    
    void loop_cntr_name()
    {
        for (int i = 0; i < 4; ++i) {
            h(i); 
        }
        sc_uint<4> i = 2;
        for (int i = 0; i < 4; ++i) {
            h(i);
        }
        i = 3;
    }
   
    void loop_cntr_name2()
    {
        sc_uint<4> i = 2;
        for (int i = 0; i < 4; ++i) {
            h(i); 
        }
        i = 3;
    }
    
    sc_signal<sc_uint<8>> s2;
    void multi_for() {
        s2 = 0;
        for (int i = 0; i < 2; i++) {
            if (s.read()) {}
        
            for (int k = 0; k < 3; k++) {
                s2 = k+2;
            }
        }
    }
    
    void do_while3() {
        int i = 0;
        do { i++; } while (i < 2);
        //while (i < 2) { i++; }

        i = 4;
    }  
    
    // Function with loop inside
    int f5() 
    {
        int res = 0;
        for (int i = 0; i < 3; i++) {
            res += 1;
        }
        return res;
    }
    
    void func_with_loop() {
        int j = f5();
    }
    
    void switch_for2() {
        int k = 0;
        switch (s.read()) {
            case 1 : 
                for (int i = 0; i < 3; i++) {
                    k = k + 1;
                }
                for (int j = 0; j < 4; j++) {
                    k = k - 1;
                }
                break;
            default : break;
        }
    }    

    void switch_if() {
        int i = 0; 
        
        switch (s.read()) {
            case 1: //break;
            case 2: 
                    if (s.read() == 1) {
                        i = 1;
                    }                       
                    i = 3;                  
                    break;
            //case 3: break;
        }
        
        i = 4;   
    }
    
    void switch_if_const() {
        int i = 0;
        switch (s.read()) {
            case 1 : break;
            case 2 : if (1 || s.read()) {
                        i = 1;
                     }
                     break;
        }
        i = 2;
    }
    
    void while_wait() 
    {
        int i = 0;
        wait();
        
        while (true) {
            while (s.read() == 42) wait();
            
            i = 1;
            wait();
            
            i = 2;
            wait();
        }
    }

    void two_do_while()    
    {
        int i = 0;
        do {
            i--;
        } while (s.read() < 1);
        
        do {
            i++;
        } while (s.read() < 2);
    }
    
    void break_in_while_for()
    {
        int k = 0;
        wait();
        
        while (1) {

            while (s.read() || t.read()) {
                if (s.read()) {
                    k = 1;
                    for (int i = 0; i < 3; i++) {
                        wait();     // 1
                        if (s.read()) break;
                    }
                    if (t.read()) {k = 3;} 
                    k = 2;
                }
                wait();     // 2
                k = 3;
            }
            k = 4;
            wait();         // 3
        }
    }
    
    void empty_loop_method() 
    {
        for (int i = 0; i < 2; ++i) {
        }
        int j = 0; 
        
        int k = 0;
        while (j < 3) {
            while (k < 4) {
                k++;
            }
            j++;
        }
    }

    void switch_if3() {
        int i;
        wait();
        
        while (true) {
            switch (s.read()) {
                case 1 : 
                case 2 : i = 2; break;
            }
            wait();
        }
    }
    
    void two_loops() 
    {
        int j = 0;
        wait();
        while (true) {
            for (int i = 0; i < 3; i++) {
                j = i;
            }
            while (true) {
                wait();                 // 1
                if (s.read()) break;
            }    
            wait();                     // 2
        }
    }
    
    void if_and_loop() 
    {
        int j = 0;
        wait();
        while (true) {
            if (s.read()) {
                for (int i = 0; i < 3; i++) {
                    j = i;
                }
            }
            while (true) {
                wait();                 // 1
                if (s.read()) break;
            }    
            wait();                     // 2
        }
    }
};


int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"b_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

