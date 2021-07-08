/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Variables name uniqueness in CTHREAD
class A : public sc_module {
public:

    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};
    
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};
    int m;
    int m1;
    int m2;
    int m3;
    int m4;
    int k;
    
    
    SC_CTOR(A) {
        SC_HAS_PROCESS(A);
        
        SC_METHOD(local_varname); sensitive << a;

        SC_CTHREAD(doble_varname_func, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(doble_varname_scope, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(doble_varname_scope_reg, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(doble_varname_scope_comb, clk.pos());
        async_reset_signal_is(rst, true);
        

        SC_CTHREAD(double_reg1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(double_reg2, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(suffix_name, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(suffix_name_next, clk.pos());
        async_reset_signal_is(rst, true);
    }
    
    // Method local variable name
    void local_varname()
    {
        int m = 1;  
        int k = 2;  
        int a;
    }
    
    // Two variables with the same name one in called function, 
    // Bug in real design in EMC
    void syncWaiting() {
        // Register
        sc_uint<3> check_hiwait_delay = a ? b.read() : c.read();
        wait();
        m = check_hiwait_delay;
    }
    void doble_varname_func() 
    {
        wait();
        
        while (true) {
            sc_uint<2> check_hiwait_delay = 1;
            m1 = check_hiwait_delay;
            
            syncWaiting();
            
            wait();
            k = m1;
        }
    }
    
    // Register and combinational
    void doble_varname_scope() 
    {
        wait();
        
        while (true) {
            sc_uint<2> acheck_hiwait_delay = 1;
            m2 = acheck_hiwait_delay;
            
            if (a) {
                // Register
                sc_uint<3> acheck_hiwait_delay = 2;
                wait(); 
                m2 = acheck_hiwait_delay; 
            }
            
            wait();
        }
    }
    
    // Two registers
    void doble_varname_scope_reg() 
    {
        wait();
        
        while (true) {
            sc_uint<2> bcheck_hiwait_delay = 1;
            wait();
            m3 = bcheck_hiwait_delay;
            
            if (a) {
                // Register
                sc_uint<3> bcheck_hiwait_delay = 2;
                wait();
                m3 = bcheck_hiwait_delay; 
            }
            
            wait();
        }
    }
    
    // Two combinational
    void doble_varname_scope_comb() 
    {
        wait();
        
        while (true) {
            sc_uint<2> ccheck_hiwait_delay = 1;
            m4 = ccheck_hiwait_delay;
            
            if (a) {
                // Register
                sc_uint<3> ccheck_hiwait_delay = 2;
                m4 = ccheck_hiwait_delay; 
            }
            
            wait();
        }
    }

//-----------------------------------------------------------------------------
    
    // Two processes with local variables with same name becomes registers
    sc_signal<bool> s1;
    void double_reg1() 
    {
        s1 = 1;
        sc_uint<3> varA;
        wait();
        
        while (true) {
            sc_bigint<4> varB = 1;
            
            if (a) {
                bool varC = b;
                wait();                     // 1
                s1 = varC ? varA : varB;
                varB = 3;
            }
            wait();                         // 2
            
            varA = varB;
            wait();                         // 3
        }
    }
    
    sc_signal<bool> s2;
    void double_reg2() 
    {
        s2 = 0;
        sc_uint<3> varA = 1;
        wait();
        
        while (true) {
            if (b) {
                sc_int<4> varB = a.read() ? 1 : 2;
                wait();                     // 1
                s2 = varB;
            }
            
            int varC = varA + 1;
            wait();                         // 2
            
            varA++;
            s2 = varC;
        }
    }
    
//-----------------------------------------------------------------------------
    // Name conflict with suffixed name

    sc_signal<int> s3;
    void suffix_name() 
    {
        sc_uint<3> varA0;
        sc_uint<4> varA1 = 42;
        sc_uint<5> varA00 = varA1;
        wait();
        
        while (true) {
            auto varB0 = varA00;
            int varA01 = 42;
            int varA02 = varA01-1;
            wait();                         // 2
            
            bool varC0 = varA0 == varB0;
            s3 = varC0 ? 1 : int(varA0 + varB0 + varA02);
            varA0 = varB0;
            wait();                         // 3
        }
    }
    
    sc_signal<int> s4;
    void suffix_name_next() 
    {
        sc_uint<3> varA00_next = 42;
        sc_uint<4> varA00 = 42;
        sc_uint<5> varB0_next = varA00;
        wait();
        
        while (true) {
            s4 = varA00_next;
            varA00_next = varB0_next; 
            varB0_next++;
            wait();                         
        }
    }
    
    
};

class B_top : public sc_module {
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    sc_clock clk_gen{"clk", sc_time(1, SC_NS)};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        clk (clk_gen);
        a_mod.clk(clk);
        a_mod.rst(rst);
        
        //SC_HAS_PROCESS(B_top);
        //SC_CTHREAD(reset_proc);
    }
    
    /*void reset_proc() {
        rst = 1;
        wait();
        
        rst = 0;
    }*/
};

int sc_main(int argc, char *argv[]) {

    B_top b_mod{"b_mod"};
//    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

