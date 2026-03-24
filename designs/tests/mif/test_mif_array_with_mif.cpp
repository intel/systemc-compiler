/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Array/vector of MIF with another MIF inside, call of nested MIF method

//------------------------------------------------------------------------------

struct Simple {
    int a;
    sc_uint<4> b;

    Simple()  {}

    bool operator ==(const Simple& other) {
        return (a == other.a && b == other.b);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, const Simple& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const Simple&, const std::string&) {}
}

//------------------------------------------------------------------------------

template <class T>
struct MIF : public sc_module, sc_interface 
{
    T              v;
    sc_signal<T>   s{"s"};
    
    SC_HAS_PROCESS(MIF);
    MIF(const sc_module_name& name) : sc_module(name)
    {}

    void setVar(T val) {
        v = val;
    }

    void setVarRef(const T& val) {
        v = val;
    }
    
    T getVar() {
        return v;
    }

    void getVar(T& val) {
        val = v;
    }
};

//------------------------------------------------------------------------------

template <class T>
struct A : public sc_module, sc_interface 
{
    sc_in<bool>     clk;
    sc_signal<bool> nrst;
    
    sc_signal<T>    sig;
    MIF<T>          mif{"mif"};
    MIF<T>          mift{"mift"};

    SC_HAS_PROCESS(A);
    A(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(fcallMeth); sensitive << sig;
        SC_CTHREAD(fcallThrd, clk.pos()); 
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<T> t0;
    void fcallMeth() 
    {
        T l;
        mif.setVar(l);          
        mif.setVarRef(l);       
        mif.setVar(sig);        
        mif.setVarRef(sig);     
        t0 = mif.getVar();
        mif.getVar(l);
        t0 = l;
    }

    sc_signal<T> t1;
    void fcallThrd() {
        T l;
        wait();
        while (true) {
            mift.setVar(l);          
            mift.setVarRef(l);       
            mift.setVar(sig);        
            mift.setVarRef(sig);     
            wait();

            t1 = mift.getVar();
            mift.getVar(l);
        }
    }
    
};

SC_MODULE(Top) 
{
    sc_in<bool>     clk;

    using T = Simple;
    sc_vector< A<T> >   arr{"arr", 2};
    
    SC_CTOR(Top) {
        arr[0].clk(clk);
        arr[1].clk(clk);
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();
    return 0;
}

