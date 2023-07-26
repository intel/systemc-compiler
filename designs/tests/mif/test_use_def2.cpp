/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// MIF unknown array element function call with parameter passed by reference
template<class T>
struct A : sc_module, sc_interface 
{   
    SC_HAS_PROCESS(A);
    A (sc_module_name name) : sc_module(name) {}
    
    sc_signal<T>  b{"b"};

    void put(const T& par) {
        b = par;
    }
    bool get(T& par) {
        par = b.read();
        return true;
    }
};

template<class T>
struct B : sc_module, sc_interface 
{   
    A<T>*   a_mod = nullptr;
    
    SC_HAS_PROCESS(B);
    B (sc_module_name name) : sc_module(name) {
        a_mod = new A<T>("a_mod");
    }
    
    void put(const T& par) {
        if (a_mod) {
            a_mod->put(par);
        }
    }
    bool get(T& par) {
        if (a_mod) {
            return a_mod->get(par);
        } else
            return false;
    }
};


template<unsigned M>
struct Top : sc_module 
{
    sc_in<bool>     clk;
    sc_signal<bool> nrst;
    
    using T = sc_uint<8>;
    B<T>   b_mod{"b_mod"};
    
    SC_CTOR(Top) 
    {
        SC_METHOD(unkwMif); sensitive << s << b_mod.a_mod->b;
        
        SC_CTHREAD(unkwMifThrd, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<unsigned> s{"s"};
    sc_signal<unsigned> t0{"t0"};
    sc_signal<unsigned> t1{"t1"};
    
    void unkwMif() {
        T a;                // removed
        b_mod.get(a);   
    }
    
    void unkwMifThrd() {
        wait();
        while(true) {
            T a;
            b_mod.put(a);   // not removed
            wait();
        }
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);
    Top<2> top("top");
    top.clk(clk);
    
    sc_start();

    return 0;
}
