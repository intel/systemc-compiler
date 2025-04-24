/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "systemc.h"
#include <iostream>

// Functions/methods calls for MIF array with record/record array fields

using namespace sc_core;
using namespace sc_dt;

struct Simple {
    bool a[3];
    int b;

    bool operator ==(const Simple& oth) {
        return (a == oth.a && b == oth.b);
    }
};

namespace std {
inline ::std::ostream& operator << (::std::ostream& os, const Simple&) 
{return os;}
}

template<class T>
struct M : public sc_module, sc_interface {
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    SC_CTOR(M) {
        SC_METHOD(meth); sensitive << c << t[1]; 
        
        SC_CTHREAD(thrd, clk.pos());     
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> c;
    sc_signal<T> t[3];
    
    T p;
    void meth() {
        T r(t[1]);                          // OK
        T p(r);
        t[1] = p; 
        
        r = t[1];
        r = p;
        t[2] = T{};                         // OK
        t[2].write(r);                      // OK
        t[c.read()].write(T{});             // OK
        t[2].write(t[1]);                   // OK
        t[c.read()].write(t[1].read());     // OK
    }
    
    T n;
    sc_signal<T> tt[3];
    void thrd() {
        T g;
        g = n;
        T h = g;
        wait();
        while (true) {
            tt[c.read()] = h;
            T j = tt[c.read()];
            c = (int)j.a[2];
            wait();
        }
    }
    
    T m;
    T read_val() {
        T r;
        r = t[1];
        r = m;
        return r;
    }

    T mmm;

    T mm;
    T read_val_() {
        T r;
        mm = t[1];
        r = mm;
        mm = r;
        return r;
    }

    int read_int() {
        return c.read();
    }
    
    T read_val2() {
        return t[1].read();
    }

    // Not supported const ref for records
    const T& read_ref2() {
        return t[1].read(); 
    }    
};

// Record (structure/class) non-module tests
template <unsigned N>
struct A : public sc_module {
    using T = Simple;

    sc_in<bool> clk;
    sc_signal<bool> nrst;
    
    sc_vector<M<T>>   SC_NAMED(mif, 2);
    
    SC_CTOR(A) {
        mif[0].clk(clk); mif[1].clk(clk);
        
        SC_METHOD(sig_ref_meth1); 
        sensitive << s << mif[0].c << mif[1].c << mif[0].t << mif[1].t;

        SC_METHOD(sig_ref_meth2); sensitive << s;

        SC_METHOD(sig_ref_meth3); 
        sensitive << s << mif[0].c << mif[1].c << mif[0].t << mif[1].t;
        
        SC_CTHREAD(sig_ref_thrd, clk.pos()); 
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> s;
    sc_signal<T> t1{"t1"};
    void sig_ref_meth1() 
    {
        T l;
        l = mif[s.read()-1].t[s.read()+1];              // OK
        mif[s.read()-1].t[s.read()+1] = T{};            // OK
        mif[s.read()-1].t[s.read()+1].write(l);         // OK
        mif[s.read()-1].t[s.read()+1].write(T{});       // OK
                
        l = mif[0].read_val();              // OK
        l = mif[1].read_val2();             // OK
        
        l = mif[s.read()].read_val();       // OK
        l = mif[s.read()].read_val2();      // OK
        
        //l = mif[0].read_ref2();           // Error -- wont support
        
        t1 = l;
    }
    
    Simple recArr[2];
    sc_signal<int> t2{"t2"};
    void sig_ref_meth2() 
    {
        int i;
        i = recArr[0].b;
        t2 = i;
    }
    
    sc_signal<T> t7{"t7"};
    void sig_ref_meth3() 
    {
        int i;
        i = mif[s.read()].mmm.b;
        t2 = i;
        
        T p;
        p = mif[s.read()].mmm;
        t7 = p;
    }
    
    sc_signal<T> t3{"t3"};
    sc_vector<sc_signal<T>> t4{"t4",2};
    void sig_ref_thrd() {
        int i;
        T q;
        T p(q);
        p = mif[0].read_val_();
        p = mif[1].read_val2();
        wait();
        
        while (true) {
            T r = p;
            T rr = p;
            t3 = r;
            p = t4[r.a[s.read()]];
            p = mif[s.read()].read_val_();
            p = mif[s.read()].read_val2();
            q = mif[0].read_val2();
            i = mif[0].read_int();
            wait();
            t4[0] = p;
            t4[1] = rr;
        }
    }
};



class Top : public sc_module {
public:
    sc_in<bool> clk;
    A<1> b_mod{"b_mod"};

    SC_CTOR(Top) {
        b_mod.clk(clk);
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 1, SC_NS};
    Top top{"top"};
    top.clk(clk);
    sc_start();
    return 0;
}

