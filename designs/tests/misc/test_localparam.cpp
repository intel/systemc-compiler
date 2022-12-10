/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// localparam generating for non-modified member variables

template <unsigned N>
struct A : public sc_module
{
    sc_in_clk               clk;
    sc_signal<bool>         nrst;
    sc_signal<unsigned>     s;
    
    SC_HAS_PROCESS(A);
    
    A(const sc_module_name& name, unsigned par) : 
        C(par), S(par+10), S1(par+10), B(par)
    {
        T = -50;
        T1 = -50;
        W = 60;
        ARR[1] = 60;
        SC_METHOD(constProc1); sensitive << s;
        SC_METHOD(constProc2); sensitive << s;
        SC_METHOD(constProc3); sensitive << s;
        SC_METHOD(constProc4); sensitive << s;
    }
    
    void set(unsigned par) {
        V = par;
        V1 = par;
        W1 = par+20;
        ARR[0] = par+20;
    }
    
    constexpr static unsigned CE = N+1;
    const unsigned C;
    sc_uint<16> S;
    sc_uint<16> S1;
    sc_int<48> T;
    sc_int<48> T1;
    unsigned V;
    unsigned V1;

    // SC types
    void constProc1() {
        S1 = 51;
        T1 = -51;
        long l;
        l = S + T;
        l = S1 + T1;
    }

    // C++ types
    void constProc2() {
        V1 = 44;
        unsigned i;
        i = CE;
        i = C;
        i = V + V1;
    }
    
    unsigned W;
    unsigned W1;
    unsigned& R = W;
    unsigned* P = &W1;
    unsigned ARR[3] = {62, 62, 62};
    
    // Pointers, references and arrays
    void constProc3() {
        unsigned i;
        i = R;
        i = *P;
        i = ARR[0] + ARR[1] + ARR[2];
    }
    
    // Conditions
    bool B;                     // localparam for single member
    bool D = 1;                 // localparam for single member
    bool E = 1;                 // localparam for single member
    bool F = 1;                 // localparam for single member
    bool MF = 1;                // No localparam for modified member
    sc_signal<bool> t;
    int ar[3] = {1, 2, 3};      // No localparam for array
    void constProc4() {
        int l = ar[2];
        if (ar[0]) l++;
        
        if (B) {
            t = 1;
        } else {
            t = 0;
        }
        t = D ? 1 : 0;
        t = E || s.read();
        t = F;
        
        t = MF;
        
        if (s.read()) MF = 0;
    }
    
};

template <unsigned N>
struct Top : public sc_module
{
     sc_in_clk  clk;
     A<N>       a{"a", 42};
     
     SC_CTOR(Top) 
    {
         a.clk(clk);
         a.set(43);
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clk{"clk", 1, SC_NS};
    Top<40> top{"top"};
    top.clk(clk);
    sc_start();

    return 0;
}


