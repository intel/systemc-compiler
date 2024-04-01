/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

// Check unused variables/statements leads to member variables removed in SV
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;

    sc_signal<int>  t1;
    sc_signal<int>  t1a;
    sc_signal<int>  t2;
    sc_signal<int>  t2a;
    sc_signal<int>  t3;
    sc_signal<int>  t3a;
    sc_signal<int>* tp;
    sc_signal<int>  sarr[3][3];
    sc_signal<int>* sparr[3][4];
    sc_vector<sc_signal<int>> vec1{"vec1", 3};
    sc_vector<sc_vector<sc_signal<int>>> vec2{"vec2", 3};

    int m1;
    int m1t;
    int m2;
    int m = 42;
    int* pm = &m;
    int* q;
    int* qt;
    int mm;
    int* qm = &mm;
    int mmt;
    int* qmt = &mmt;
    sc_uint<12> x;
    sc_uint<12>* px = &x;
    int* parr[3];
    int arr[3][3];
    int arrt[3][3];
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name) 
    {
        q = sc_new<int>();
        qt = sc_new<int>();
        tp = new sc_signal<int>("tp");
        
        for (int i = 0; i < 3; i++) {
            vec2[i].init(4);
            parr[i] = sc_new<int>();
            
            for (int j = 0; j < 4; j++) {
                sparr[i][j] = new sc_signal<int>("parr");
            }
        }
        
        SC_METHOD(local_pointer_bug); sensitive << s;
        SC_METHOD(local_pointer_bug2); sensitive << s;
        
        SC_METHOD(remove_member1); sensitive << s;
        SC_METHOD(remove_member1a); sensitive << s;
        SC_METHOD(remove_member2); sensitive << s;
        SC_METHOD(remove_member2a); sensitive << s;
        SC_METHOD(remove_member3); sensitive << s;
        SC_METHOD(remove_member3a); sensitive << s;
        SC_METHOD(remove_member4); sensitive << s << sarr[0][0] << *sparr[0][0] << vec2[0][0];
        SC_METHOD(remove_member4a); sensitive << s << vec1[0];
        
        SC_CTHREAD(remove_member1_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member1a_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member2_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member2a_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member3_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member3a_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member4_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member4a_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(remove5, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

 // --------------------------------------------------------------------------   
    
    // Statement with pointer defined cannot be removed -- fixed
    sc_signal<int> t0a;
    void local_pointer_bug() {
        int* lp = pm;
        int k = *lp;
        t0a = k;
    }

    // Statement write through pointer cannot be removed
    int tt;
    int* tm = &tt;
    sc_signal<int> t0b;
    void local_pointer_bug2() {
        int* lt = tm;
        *lt = 42;
        t0b = tt;
    }
    
 // --------------------------------------------------------------------------    
    
    // General variable
    void remove_member1() {
        int k = 1;
        m1 = 1;
        k = m1 + 1;
        t1a = m1;
    }

    void remove_member1_thread() {
        int k = 1;
        wait();

        while (true) {
            m1t = 1;
            k = m1t + 1;
            t1 = m1t;
            wait();
        }
    }
    
    // All the variables removed
    void remove_member1a() {
        int k = 1;
        m2 = 1;
        k = m2 + 1;
    }
    
    void remove_member1a_thread() {
        int k = 1;
        m2 = 1;
        k = m2 + 1;
        wait();

        while (true) {
            wait();
        }
    }
    
    // Pointer to variable/dynamic memory
    void remove_member2() {       
        int* lp2 = q;
        int l = (*lp2)--;
    }
    
    sc_signal<int> t0c;
    void remove_member2_thread() {
        int* lp = qm;
        mm = 1;
        wait();

        while (true) {
            (*lp)++;
            t0c = *lp;

            mmt = 1;
            t0c = (*qmt)++;
            wait();

            int* lp2 = qt;
            int l = (*lp2)--;
        }
    }    
    
    // All the variables removed
    void remove_member2a() {
        x = 1;
        int k = 2 + x;
        k = *px;
    }
    
    void remove_member2a_thread() {
        wait();

        while (true) {
            x = 1;
            int k = 2 + x;
            k = *px;
            wait();
        }
    }    
    
    // Array 
    void remove_member3() {
        t2a = arr[s.read()][s.read()];
    }
    
    void remove_member3_thread() {
        wait();

        while (true) {
            t2 = arrt[s.read()][s.read()];
            wait();
        }
    }    
    
    
    // All the variables removed
    void remove_member3a() {
        int l = *parr[s.read()];
    }
    
    void remove_member3a_thread() {
        wait();

        while (true) {
            int l = *parr[s.read()];
            wait();
        }
    }    
    
    
    // Signal array and vector
    void remove_member4() {
        int ll = sarr[s.read()][s.read()+1];
        ll += *sparr[s.read()][s.read()+1];
        int k = vec2[s.read()][s.read()+1];
        t3a = k + ll;
    }
    
    void remove_member4_thread() {
        wait();

        while (true) {
            int ll = sarr[s.read()][s.read()+1];
            ll += *sparr[s.read()][s.read()+1];
            int k = vec2[s.read()][s.read()+1];
            t3 = k + ll;
            wait();
        }
    }    
    
    
    // All the variables removed
    void remove_member4a() {
        int l = vec1[s.read()];
    }
    
    void remove_member4a_thread() {
        wait();

        while (true) {
            int l = vec1[s.read()];
            wait();
        }
    }    
    
    
    // Assertions
    sc_signal<bool> a0;         // removed
    sc_signal<bool> a1;
    sc_signal<int> a2;
    sc_signal<int> a3;
    sc_signal<sc_uint<33>> a4;
    sc_signal<sc_int<33>> a5;   // removed
    void remove5() 
    {
        int i = a0.read();
        int j = a3.read();
        int k = j+1;
        
        SCT_ASSERT_THREAD(a1, SCT_TIME(1), a2, clk.pos());
        wait();
        
        SCT_ASSERT_THREAD(k == 42, (1,2), a2 == 42, clk.pos());
        
        while (1) {
            
            int l = a4.read();
            
            wait();

            sct_assert(l == s.read());
        }
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

