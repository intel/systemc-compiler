/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

constexpr static unsigned L[3][2] = {{42, 43}, {44, 45}, {46, 47}};
const unsigned LL[3][2] = {{42, 43}, {44, 45}, {46, 47}};

// Constant, constant array, constexpr static and static array members 
// in module and MIF array
struct M : public sc_module, sc_interface 
{
    const static int K = 42;
    constexpr static unsigned G[2] = {42, 43};
    const unsigned GG[2] = {42, 43};
    constexpr static unsigned P[3][2] = {{42, 43}, {44, 45}, {46, 47}};
    const unsigned PP[3][2] = {{42, 43}, {44, 45}, {46, 47}};
    
    int getG(int indx) {
        return G[indx];
    }

    M(const sc_module_name& name) : sc_module(name) {}
};    

template<unsigned N>
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;

    const bool  C1 = true;
    const bool  C2 = false;
    const bool  arr[3] = {true, true, false};
    constexpr static int B = 42;
    constexpr static int E[3] = {-41, 42, 43};
    constexpr static unsigned D[2] = {42, 43};
    const unsigned DD[2] = {42, 43};

    constexpr static unsigned S[3][2] = {{42, 43}, {44, 45}, {46, 47}};
    const unsigned SS[3][2] = {{42, 43}, {44, 45}, {46, 47}};

    sc_vector<M>  mif{"mif", 2};
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD(array_method); sensitive << s;

        SC_METHOD(array_method2); sensitive << s;
        
        SC_CTHREAD(array_thrd, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(mif_method); sensitive << s;

        SC_CTHREAD(mif_thrd, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
//-----------------------------------------------------------------------------    

    sc_signal<int> t0;
    void array_method() 
    {
        const bool larr[3] = {true, true, false};
        
        bool b1 = arr[1];
        b1 = larr[1];
        bool b2 = C1;
        b2 = C2;
        t0 = b1 + b2;
    }
    
    sc_signal<int> t1;
    void array_method2() 
    {
        int l;
        l = D[s.read()-1];      // OK
        l = DD[s.read()+1];     // OK
        l = L[0][s.read()];
        l = LL[s.read()][0];
        t1 = l;
        
        t1 = B + E[1];          // OK
    }
    
    sc_signal<int> t2;
    void array_thrd() {
        int i = E[1];
        t2 = i + D[1] + S[0][0] + SS[1][1];
        wait();
        while(true) {
            t2 = E[s.read()];
            wait();
            t2 = C1 + D[s.read()];
            t2 = S[s.read()-1][s.read()+1];
            t2 = SS[s.read()-1][s.read()+1];
        }
    }
//-----------------------------------------------------------------------------    
    
    sc_signal<int> t3;
    void mif_method() 
    {
        int l;
        l = mif[1].K;
        l = mif[1].G[0] + M::G[0];
        l = mif[1].GG[0];

        l = mif[s.read()].G[0];
        l = mif[s.read()].GG[0];
        
        l = mif[s.read()].P[0][s.read()];
        l = mif[s.read()].PP[s.read()-1][s.read()+1];

        l = mif[s.read()].G[s.read()-1];
                
        l = mif[s.read()].getG(s.read()); 
        t3 = l;
        
        t3 = mif[0].G[1];
    }
    
    sc_signal<int> t4;
    void mif_thrd() {
        t4 = mif[0].getG(1);
        t4 = mif[1].G[0];
        t4 = mif[1].getG(0);
        wait();
        while(true) {
            unsigned l;
            l = mif[s.read()].getG(s.read());
            l = l + mif[1].getG(s.read());

            l = mif[s.read()].P[0][s.read()];
            l = mif[s.read()].PP[s.read()-1][s.read()+1];
            t4 = l;
            
            t4 = mif[1].G[s.read()];
            t4 = mif[s.read()].G[s.read()+1];
            t4 = mif[s.read()].getG(s.read()+1);
            wait();
        }
    }
//    
    
};

int sc_main(int argc, char *argv[]) 
{
    A<2> a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

