/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Record array local declaration in IF/loop/other scopes in METHOD and CTHREAD
class A : public sc_module {
public:
    sc_in<bool>     clk;
    sc_signal<bool> nrst{"nrst"};
    sc_signal<int>  sig{"sig"};


    SC_CTOR(A) 
    {
        SC_METHOD(rec_arr_if_meth);
        sensitive << sig;
        
        SC_METHOD(rec_arr_loop_meth);
        sensitive << sig;

        SC_METHOD(rec_arr_compl_meth);
        sensitive << sig;

        SC_CTHREAD(rec_arr_if_thread_comb, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_if_thread_reg, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_if_thread_reg2, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_if_thread_reg, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        // #141
        //SC_CTHREAD(rec_arr_if_thread_reg2, clk.pos());  
        //async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_if_loop_thread, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_if_loop_thread_break, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_if_loop_thread_reset, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
    }
    
    struct Simple {
        sc_int<2>  a;
        sc_uint<4> b;
    };

    bool f1(Simple par) {
        return (par.a > par.b);
    }
    
    // Record array declared in IF scope METHOD
    void rec_arr_if_meth()
    {
        int i = sig.read();
        
        if (i == 1) {
            Simple s;
            
            s.a = 1;
            s.b = i+1;

            if (f1(s)) {
                Simple r[3];
                r[s.b].a = s.a++;
            }
        }
    }
    
    // Record array declared in loop scope METHOD
    void rec_arr_loop_meth()
    {
        int i = 0;
        while (i < 10) {
            Simple s[3];
            s[2].a = i;

            unsigned k = 0;
            for (int j = 0; j < 3; j++) {
                s[j].b = s[j].a + j;
            }
            i++;
        }
    }
    
    // Record array declared in if/loop scope METHOD
    void rec_arr_compl_meth()
    {
        for (int i = 0; i < 3; i++) {
            Simple s[3];
            s[i].b = i;
            
            for (int j = 0; j < 3; j++) {
                if (i == j) {
                    Simple r[3];
                    r[1] = s[j];
                    
                    Simple s;
                    s.a = j;
                }
            }
        }
    }
    
//----------------------------------------------------------------------------    
    
    // Record array declared in IF scope CTHREAD
    void rec_arr_if_thread_comb()
    {
        int i = sig.read();
        wait();
        
        while(true) {
            if (i == 1) {
                Simple s;           // comb
                s.b = i+1;

                if (f1(s)) {
                    Simple r[3];    // comb
                    r[i].a = s.a;
                }
            }
            wait();
            
            i++;
        }
    }
    
    // This test used to debug #97, in result sub-values removing added
    void rec_if_thread_reg()
    {
        int i = sig.read();
        wait();
        
        while (true) {
            if (i) {
                Simple r;
                
                wait();
                
                r.a = 1;
            }
            wait();
        }
    }
    
    
    // This test used to debug #97, in result sub-values removing added
    void rec_if_thread_reg2()
    {
        int i = sig.read();
        wait();
        
        while (true) {
            if (i) {
                Simple r;
                
                wait();
                
                if (i) {
                    i++;
                }
                
                r.a = 1;
            }
            wait();
        }
    }

    
    // Record and record array registers
    void rec_arr_if_thread_reg()
    {
        int i = sig.read();
        wait();
        
        while (true) 
        {
            if (i == 1) {
                Simple r[3];   // reg
                Simple s;      // reg
                s.b = i+1;

                if (i == 2) {
                    i = r[i].b;
                    wait();   // 1
                }
                wait();       // 2 
                
                r[i].a =  s.a;
            }
            wait();           // 3
        }
    }
    
    // Several IFs with wait() inside 
    void rec_arr_if_thread_reg2()
    {
        int i = sig.read();
        wait();
        
        while (true) 
        {
            Simple e[3];   // reg
            
            if (i == 1) {
                wait();        // 1
                Simple d[3];   // not used
                i++;
                
            } else {
                Simple f[3];   // reg
                f[1] = e[2];
                
                wait();        // 2
                
                if (i == 2) {
                    i = f[i].a;
                    
                } else {
                    wait();    // 3
                }
                
                wait();        // 4
                i = e[i+1].b;
            }
        }
    }    
    
    // Several IFs and loops with wait() inside 
    void rec_arr_if_loop_thread()
    {
        int i = sig.read();
        wait();
        
        while (true) 
        {
            while (!sig.read()) {
                
                Simple g[3];
            
                wait();             // 1
                
                if (i) {
                    g[i].a = i+1;
                    wait();         // 2
                }
                
                for (int i = 0; i < 3; i++) {
                    g[i].b = i+1;
                }
                
                i = g[i].a;
            }
            
            wait();                 // 3
        }
    }
    
    // Several IFs and loops with @break and wait() inside 
    void rec_arr_if_loop_thread_break()
    {
        int i = sig.read();
        wait();
        
        while (true) 
        {
            Simple m[3];
            
            while (true) {
                Simple h[3];

                if (sig.read()) break;

                i = h[i].a;
                
                wait();         // 1
            }
            
            m[i+1].b = i; 
            
            wait();             // 2
        }
    }
    
    // Several IFs and loops in reset
    void rec_arr_if_loop_thread_reset()
    {
        Simple n[3];        // reg
        for (int i = 0; i < 3; i++) {
            n[i].b = i;
        }
        int j = sig.read();
        if (j) {
            Simple p[3];    // comb
            p[j].b = n[1].a;
        }
        wait();
        
        while (true) 
        {
            j = n[j].b;
            wait();
        }
    }
    
};

class B_top : public sc_module {
public:
    sc_clock  clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

