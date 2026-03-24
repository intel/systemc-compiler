/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include <systemc.h>

// Local variable and record declaration in multi-state thread 

struct ScSimple 
{
    int a = 1;
    sc_uint<2> b;
    inline friend bool operator==(const ScSimple& lhs, const ScSimple& rhs)
    {return (lhs.a == rhs.a && lhs.b == rhs.b);}
    inline friend std::ostream& operator<< (std::ostream& os, const ScSimple &obj)
    {return os;}
    inline friend void sc_trace(sc_trace_file*& f, const ScSimple& val, std::string name) 
    {}
};

class top : sc_module
{
public:
    sc_in<bool> clk;
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> s{"s"};
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(comb_var_with_assert, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(comb_var_in_state1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(comb_var_in_state2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(fcall_var_in_state1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(fcall_var_in_state2, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(comb_rec_in_state, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(comb_arr_in_state, clk.pos());
        async_reset_signal_is(arstn, false);
    }
    
    // Thread with assertion after reset should not considered as multi-state
    sc_signal<int> t7;
    void comb_var_with_assert()
    {
        t7 = 0;                 
        wait();
        
        if (true) {
            SCT_ASSERT_THREAD(true, clk.pos());
        }
        
        while (true) {
            int b = 1;         // OK, no initialization for @b 
            t7 = b;             
            wait();
        }
    }    
    
    // Local variable declared and used in states
    unsigned VAR = 42;
    sc_signal<int> t1;
    void comb_var_in_state1()
    {
        int a = 0;              // OK
        t1 = a;                 
        wait();
        
        while (true) {
            int b = 1;          // OK
            t1 = b;
            t1 = VAR;           // Read-only -- localparam
            wait();
            
            sc_uint<16> c = 2;  // OK
            t1 = c;
            
            sc_uint<16> d = 3;  // Unused variable -- removed
            wait();
        }
    }
    
    // Local variable declared and used in inner scope of states
    sc_signal<int> t3;
    void comb_var_in_state2()
    {
        t3 = 0;
        wait();
        
        while (true) {
            if (s.read()) {
                int b = 1;          
                t3 = b;
            }
            wait();
            
            for (int i = 0; i < 10; ++i) {
                sc_uint<16> c = i;  
                t3 = c;
                wait();
            }
        }
    }
    

    // Local variable is function parameter or returned temporary variable
    template<class T>
    int f0(T par_) {
        return (par_+1);
    }
    
    template<class T>
    int f1(T par) {
        return (par+1);
    }

    template<class T>
    int f2(T& par) {
        return (par+1);
    }
    
    sc_signal<int> t2;
    void fcall_var_in_state1()
    {
        int a = s.read();
        t2 = f0(a);
        wait();
        
        while (true) {
            int b = s.read();
            t2 = f1(b);
            wait();
            
            sc_uint<16> c = 2;
            t2 = f2(c);
            wait();
        }
    }
    
    // Function parameter/returned variable in inner scope of states
    sc_signal<int> t4;
    void fcall_var_in_state2()
    {
        t4 = 0;
        wait();
        
        while (true) {
            if (s.read()) {
                int b = s.read();
                t4 = f1(b);
            }
            wait();
            
            for (int i = 0; i < 10; ++i) {
                sc_uint<16> c = i;
                t4 = f2(c);
            }
            wait();
        }
    }
    
//-----------------------------------------------------------------------------

    // Local record declared and used in states
    sc_signal<ScSimple> t5;
    sc_signal<int> t5a;
    void comb_rec_in_state()
    {
        wait();
        
        while (true) {
            ScSimple rec1;
            t5a = rec1.a;
            wait();
            
            ScSimple rec2;
            rec2.b = 2;
            t5 = rec2;
            wait();
        }
    }
    
//-----------------------------------------------------------------------------
    
    // Local array declared and used in states
    sc_signal<int> t6;
    void comb_arr_in_state()
    {
        int a[] = {1,2,3}; 
        t6 = a[2];
        wait();
        
        while (true) {
            int b[2] = {4,5};
            t6 = b[s.read()];
            wait();
            
            sc_uint<16> c[3];
            t6 = c[1] + c[s.read()];
            wait();
        }
    }    
 };

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

