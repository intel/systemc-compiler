/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// MIF unknown array element function call with parameter passed by reference
// Multiple parameters and different array elements
template<class T>
struct A : sc_module, sc_interface 
{   
    A (sc_module_name name) : sc_module(name)
    {}
    
    T a;
    sc_signal<T>  b{"b"};

    bool compare(T& par1, T& par2) {
        bool b = par1 == par2 ? 1 : 2;
        return b;
    }
    void set(const T& par1, const T& par2) {
        b = par1 + par2;
    }
    void set(const T& par) {
        b = par + 1;
    }
    void def() {
        T k;
        b = k;
    }
};

template<class T>
struct Simple {
    bool a;
    T b;
    
    bool operator == (const Simple<T>& other) {
        return (a == other.a && b == other.b);
    }
    
    bool compare(T& par1, T& par2) {
        bool b = par1 == par2 ? 1 : 2;
        return b;
    }
    void set(const T& par1, const T& par2) {
        b = par1 + par2;
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
    
    SC_CTOR(Top) 
    {
        SC_METHOD(unkwMif); sensitive << s;
        SC_METHOD(unkwMulti); sensitive << s << t;
        SC_METHOD(unkwRec); sensitive << s << t;
        SC_METHOD(unkwMem); sensitive << s << t;
    }
    
    sc_signal<unsigned> s{"s"};
    sc_signal<unsigned> t{"t"};
    sc_signal<unsigned> t0{"t0"};
    sc_signal<unsigned> t1{"t1"};
    sc_signal<unsigned> t2{"t2"};
    
    void unkwMif() {
        unsigned port = s.read();
        T a = s.read();
        T b = s.read()+1;
        mif[port].compare(a, b);
        
        T c = s.read();
        mif[port].compare(c, c);

        T d = s.read();
        T e = s.read()+1;
        bool res = mif[port].compare(d, e);
        t0 = res; 
        
        mif[port].def();
    }
    
     void unkwMulti() {
        unsigned port = s.read();
        mif[port].set(mif[port+1].a);

        T b = t.read();
        mif[port].compare(mif[s.read()].a, b); 
        mif[port].compare(mif[s.read()].a, mif[s.read()].a);
        mif[port].set(mif[port-1].a, mif[port+1].a);
    }
    
    void unkwRec() {
        Simple<T> ss[2];
        unsigned port = s.read();
        T a = s.read();
        T b = t.read();
        ss[port].compare(a, b);

        T c;
        ss[port].set(ss[port].b, c);        
    }

    T m1;
    T m2;
    T m3;
    Simple<T> mm[2];
    void unkwMem() {
        unsigned port = s.read();
        m1 = s.read();
        m2 = s.read();
        m3 = s.read();
        
        mif[port].compare(m1, m2);
        mif[port].set(m3, mm[t.read()].b);
        
        mm[t.read()].compare(mm[port].b, m2);
        mm[port].set(m3, mm[t.read()].b);
    }
    
};

int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);
    Top<2> top("top");
    
    sc_start();

    return 0;
}
