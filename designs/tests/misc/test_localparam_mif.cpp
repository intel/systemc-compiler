/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// localparam generating for non-modified member variables in MIF array

struct D : public sc_module, sc_interface
{
    unsigned T;
    D(const sc_module_name& name) : sc_module(name){
        T = 41;
    }
};

struct A : public D
{
    sc_signal<unsigned>     s;
    
    SC_HAS_PROCESS(A);
    
    A(const sc_module_name& name) : D(name)
    {
        S = 42;
        
        SC_METHOD(childProc); sensitive << s;
    }

    const bool BC = true;
    bool B;
    unsigned S;

    void childProc() {
        int l = S;
    }
    
    bool f() {
        return B;
    }
    
    bool g() {
        return ((s.read() == B) ? s.read() : B);
    }
};

template <unsigned N>
struct Top : public sc_module
{
    sc_in_clk  clk;
    sc_signal<bool>    t;

    A               a{"a"};
    sc_vector<A>    aa{"aa", 2};
    
     
    SC_CTOR(Top) 
    {
        aa[0].B  = 0;
        aa[1].B  = 1;
         
        SC_METHOD(topProc); sensitive << t;
    }
    
    void topProc() {
        int l;
        int i = t.read();
        l = a.T;
        l = aa[i].BC;
        l = aa[i].B;
        for (int i = 0; i < 2; ++i) {
            l = t.read() || aa[i].g();
        }
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clk{"clk", 1, SC_NS};
    Top<40> top{"top"};
    top.clk(clk);
    sc_start();

    return 0;
}


