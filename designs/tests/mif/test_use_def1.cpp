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
    
    A (sc_module_name name) : sc_module(name)
    {
        SC_METHOD(unkwLoc); sensitive << b;
    }
    
    sc_signal<T>  b{"b"};
    sc_signal<T>  c{"c"};

    void put(const T& par) {
        b = par;
    }
    void get(T& par) {
        par = b;
    }
    
    void unkwLoc() {
        const T& l = b.read();
        c = l;
    }
};

template<class T>
struct Simple {
    bool a;
    T b;
    
    bool operator == (const Simple<T>& other) {
        return (a == other.a && b == other.b);
    }
    
    void put(const T& par) {
        b = par;
    }
    void get(T& par) {
        par = b;
    }
};
template<class T>
inline ::std::ostream& operator << (::std::ostream& os, const Simple<T>& s) {
    return os;
}

template<unsigned M>
struct Top : sc_module, sc_interface 
{
    using T = sc_uint<8>;
    sc_vector<A<T>> mif{"mif", M};
    sc_vector<sc_vector<A<T>>> mif2d{"mif2d", M};
    
    SC_CTOR(Top) 
    {
        for (unsigned i = 0; i < M; ++i) mif2d[i].init(M);
        SC_METHOD(unkwMif); sensitive << s;
        for (unsigned i = 0; i < M; ++i) sensitive << mif[i].b;
        SC_METHOD(unkwMif2D); sensitive << s;
        for (unsigned i = 0; i < M; ++i) 
            for (unsigned j = 0; j < M; ++j) sensitive << mif2d[i][j].b;
        SC_METHOD(unkwRec); sensitive << s;
        SC_METHOD(unkwRec2D); sensitive << s;
        SC_METHOD(unkwMem); sensitive << s;
    }
    
    sc_signal<unsigned> s{"s"};
    sc_signal<unsigned> t0{"t0"};
    sc_signal<unsigned> t1{"t1"};
    sc_signal<unsigned> t2{"t2"};
    sc_signal<unsigned> t3{"t3"};
    sc_signal<unsigned> t4{"t4"};
    
    void unkwMif() {
        unsigned port = s.read();
        T a = s.read();
        mif[port].put(a);
        T b;
        mif[port].get(b);
        t0 = b;
    }
    
    void unkwMif2D() {
        unsigned port = s.read();
        T a = s.read();
        mif2d[port][port-1].put(a);
        T b;
        mif2d[port][a].get(b);
        t1 = b;
    }

    void unkwRec() {
        Simple<T> ss[2];
        unsigned port = s.read();
        T a = s.read();
        ss[port].put(a);
        T b;
        ss[port].get(b);
        t2 = b;
    }
    
    void unkwRec2D() {
        Simple<T> ss[2][2];
        unsigned port = s.read();
        T a = s.read();
        ss[port][port+1].put(a);
        ss[1][port].put(a);
        ss[port][1].put(a);
        T b;
        ss[port][a].get(b);
        t3 = b;
        ss[port][0].get(b);
        t3 = b;
        ss[0][port].get(b);
        t3 = b;
    }

    T m1;
    T m2;
    T m3;
    T m4;
    void unkwMem() {
        Simple<T> ss[2];
        unsigned port = s.read();
        m1 = s.read();
        m2 = s.read();              // removed
        m3 = s.read();
        
        mif[port].put(m1);
        mif[port].get(m2);
        
        ss[port].put(m3);
        ss[port].get(m4);
        t4 = m4;
    }
    
};

int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);
    Top<2> top("top");
    
    sc_start();

    return 0;
}
