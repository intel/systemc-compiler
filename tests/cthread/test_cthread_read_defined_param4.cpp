/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Use/Def analysis for function calls with arrays and pointers parameters 
// used after wait()
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    static const unsigned ARR[3];
    static const int ARRI[3];
    sc_signal<int>      s;
    
    sc_uint<3>* pi;
    sc_uint<3>* pj;
    sc_uint<3>* pk;
    sc_uint<3>* pl;
    
    sc_signal<bool>* pc;
    sc_signal<int>*  parr;
    sc_signal<int>*  parr_;

    SC_CTOR(A)
    {
        pi = sc_new<sc_uint<3>>();
        pj = sc_new<sc_uint<3>>();
        pk = sc_new<sc_uint<3>>();
        pl = sc_new<sc_uint<3>>();
        
        pc = new sc_signal<bool>("pc");
        parr = sc_new_array<sc_signal<int>>(3);
        parr_ = sc_new_array<sc_signal<int>>(3);
        
        SC_METHOD(const_ref_call); sensitive << s;
        
        SC_CTHREAD(const_ref_call1, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(const_ref_call2, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(const_ref_call3, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(const_ref_call4, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(array_thread1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(array_thread2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(array_thread3, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(array_thread4, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(array_thread5, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(chan_array_thread1, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(chan_array_thread2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(ptr_thread1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(ptr_thread2, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(ptr_ch_thread1, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
   
// ------------------------------------------------------------------------
    // Array element passed via constant reference
    
    unsigned cmref(const unsigned& par) {
        unsigned l;
        l = par;
        return l;
    }
    
    const unsigned E = 1;
    void const_ref_call() {
        unsigned j = 0;
        j = cmref(E);
        j = cmref(ARR[s.read()]) + cmref(ARR[1]);
    }        
    
    unsigned cref(const unsigned& par) {
        unsigned l;
        wait();             // 1
        l = par;
        return l;
    }

    void const_ref_call1() {
        unsigned j = 0;
        wait();             // 0
        
        while (true) {
            j = cref(ARR[2]);
            wait();         // 2
        }
    }
    
    void const_ref_call2() {
        unsigned j = 0;
        wait();             // 0
        
        while (true) {
            j = cref(ARR[s.read()]);  // Incorrect code generated, see #182
            wait();         // 2
        }
    }
    
    void const_ref_call3() {
        unsigned j = 0;
        wait();             // 0
        
        while (true) {
            unsigned arrc[3]; 
            for(int i = 0; i < 3; i++) arrc[i] = i;
            j = cref(arrc[2]);

            wait();         // 2
        }
    }
    
    unsigned arrcm[3]; 
    void const_ref_call4() {
        unsigned j = 0;
        wait();             // 0
        
        while (true) {
            for(int i = 0; i < 3; i++) arrcm[i] = i;
            j = cref(arrcm[s.read()+1]);  // Incorrect code generated, see #182

            wait();         // 2
        }
    }
    
// ------------------------------------------------------------------------

    // Array passed into function with wait() 
    int arr_wait1(int* par) {
        wait();
        par[s.read()] = 2;
        int l = par[s.read()];      // @larr is register anyway
        return l;
    }
    
    void array_thread1() {
        int j;
        int larr[3] = {1, 2, 3};
        wait();
        
        while (true) {
            j = arr_wait1(larr);
            wait();
        }
    }
    
    int marr[3];
    void array_thread2() {
        int j;
        wait();
        
        while (true) {
            j = arr_wait1(marr);
            wait();
        }
    }
    
// ------------------------------------------------------------------------

    // Array passed to function via constant pointer
    int arr_wait2(const int* par) {
        wait();
        int l = par[s.read()] + par[1];
        return l;
    }

    void array_thread3() {
        int j;
        wait();
        
        while (true) {
            j = arr_wait2(ARRI);
            wait();
        }
    }
    
    void array_thread4() {
        int j;
        int llarr[3] = {1, 2, 3};
        wait();
        
        while (true) {
            j = arr_wait2(llarr);
            wait();
        }
    }
    
    int mmarr[3];
    void array_thread5() {
        int j;
        for(int i = 0; i < 3; i++) mmarr[i] = 0;
        wait();
        
        while (true) {
            j = arr_wait2(mmarr);
            wait();
            
            for(int i = 0; i < 3; i++) mmarr[i] = i;
        }
    }
    
// ------------------------------------------------------------------------

    // Channel dynamic array
    template<typename T>
    int chan_arr_wait1(T par[]) {
        wait();
        par[1] = 42;
        int l = par[s.read()];
        return l;
    }
    
    void chan_array_thread1() {
        int i;
        wait();
        
        while (true) {
            i = chan_arr_wait1(parr);
            wait();
        }
    }

    int chan_arr_wait2(sc_signal<int>* par) {
        wait();
        par[1] = 42;
        int l = par[s.read()];
        return l;
    }
    
    void chan_array_thread2() {
        int i;
        wait();
        
        while (true) {
            i = chan_arr_wait2(parr_);
            wait();
        }
    }
    
// ------------------------------------------------------------------------
    // Pointers
    
    // Pointer passed into function with wait()     
    sc_uint<4> fptr2(sc_uint<3>* val) {
        wait();             // 1
        int l = *val;
        return l;
    }
    
    // @pk not register
    void ptr_thread1() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            *pk = 1;
            fptr2(pk);
            wait();         // 2
        }
    }
    
    // @pl register
    void ptr_thread2() {
        int i = 0;
        *pl = 1;
        wait();             // 0
        
        while (true) {
            fptr2(pl);
            wait();         // 2
        }
    }
    
    // Channel pointer
    bool fch_ptr(sc_signal<bool>* p) {
        wait();
        bool ll = p->read();
        wait();
        *p = !ll;
        return ll;
    }
    
    void ptr_ch_thread1() 
    {
        *pc = 0;
        wait();         
        
        while (true) {
            bool d = fch_ptr(pc);
            wait();     
        }
    }
    
};

const unsigned A::ARR[3] = {1,2,3};
const int A::ARRI[3] = {2,3,4};

class B_top : public sc_module
{
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

