/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// References/constant references and pointers as function parameters in CTHREAD
class A : public sc_module {
public:
    SC_HAS_PROCESS(A);

    sc_in_clk clk;
    sc_signal<bool> nrst;
    sc_signal<bool> s;

    sc_out<bool>        out{"out"};
    sc_out<bool>*       p;
    
    int                 m;
    int*                q1;
    int*                q2;
    int*                q3;
    
    A(const sc_module_name& name) : sc_module(name)
    {
        p = new sc_out<bool>("p");
        q1 = sc_new<int>();
        q2 = sc_new<int>();
        q3 = sc_new<int>();
        
        SC_CTHREAD(fcall_ref_reset1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(fcall_ref_reset2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(fcall_ref1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(fcall_ref2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(fcall_ptr1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(fcall_ptr2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(fcall_ptr3, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(fcall_rec_ref1, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(fcall_rec_ref2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
// ---------------------------------------------------------------------------
    
    
    void ref(sc_uint<4>& val) {
        sc_uint<4> k = val.range(2,1);
        val = k+1;
    }

    sc_uint<2> const_ref(const sc_uint<4>& val) {
        sc_uint<2> n = val.range(1,0);
        return (n+1);
    }
    
    template <class T>
    T const_ref_tmp(const T& val) {
        T l = val;
        return l;
    }

    sc_signal<int> s0;
    void fcall_ref_reset1()
    {
        sc_uint<4> i = 42;      // comb   
        ref(i);
        sc_uint<4> j = i;       // comb
        s0 = const_ref(j);
        wait();
        
        while (true) {
            wait();
        }
    }
    
    sc_signal<int> s1;
    void fcall_ref_reset2()
    {
        sc_uint<4> i = 42;      // reg   
        int j = i+1;            // reg
        wait();
        
        while (true) {
            ref(i);
            s1 = i;
            s1 = const_ref(j);
            // Materialize temporary expression here
            int a = 42;         // comb
            s1 = const_ref(a);
            wait();
        }
    }
    
    sc_signal<int> s2;
    void fcall_ref1()
    {
        wait();
        
        while (true) {
            sc_uint<4> i = 42;  // comb
            ref(i);
            int j = i+1;        // comb
            s2 = const_ref_tmp(j);
            
            wait();
        }
    }

    sc_signal<int> s3;
    void fcall_ref2()
    {
        wait();
        
        while (true) {
            sc_uint<4> i = 42;  // comb
            int j = i+1;        // comb
            
            wait();

            if (s.read()) {
                s3 = const_ref_tmp(j);
            } else {
                ref(i);
            }
        }
    }
    
// ---------------------------------------------------------------------------
    
    // Pointer parameter
    int ptr1(int* val) {
        return (*val + 1);
    }

    // Pointer parameter modified inside
    void ptr2(int* val) {
        int loc = *val + 1;
        *val = loc;
    }

    
    // Function with pointer parameter
    sc_signal<int> s5;
    void fcall_ptr1()
    {
        wait();
        
        while (true) {
            *q1 = 1;            // comb
            s5 = ptr1(q1);
            ptr2(q1);
            s5 = *q1;
            
            wait();
        }
    }
    
    sc_signal<int> s6;
    void fcall_ptr2()
    {
        wait();
        
        while (true) {
            *q2 = 1;            // reg
            wait();

            s6 = ptr1(q2);
        }
    }

    sc_signal<int> s7;
    void fcall_ptr3()
    {
        wait();
        
        while (true) {
            *q3 = 1;            // reg
            wait();

            ptr2(q3);
            s7 = *q3;
        }
    }

    
// ----------------------------------------------------------------------------
    // Recursive reference
    
    void rec_ref_(sc_uint<4>& par_) 
    {
        par_++;
    }
    
    sc_uint<4> rec_ref(sc_uint<4>& par) 
    {
        rec_ref_(par);
        return par + 1;
    }
    
    sc_signal<int> s8;
    void fcall_rec_ref1()
    {
        wait();
        
        while (true) {
            sc_uint<4> x = s.read();        // reg
            wait();

            s8 = rec_ref(x);
        }
    }
    
// ----------------------------------------------------------------------------
    // Recursive constant reference
    
    int rec_ref_const_(const sc_uint<4>& par_) 
    {
        return par_ + 1;
    }
    
    sc_uint<4> rec_ref_const(const sc_uint<4>& par) 
    {
        return rec_ref_const_(par) + 1;
    }
    
    sc_signal<int> s9;
    void fcall_rec_ref2()
    {
        wait();
        
        while (true) {
            s9 = rec_ref_const(s.read());
            
            wait();
        }
    }

};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<bool>      s1;
    sc_signal<bool>      s2;

    A a_mod{"a_mod"};
    a_mod.clk(clk);
    a_mod.p->bind(s1);
    a_mod.out(s2);
    
    sc_start();
    return 0;
}

