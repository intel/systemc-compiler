/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "systemc.h"
#include <iostream>

// Functions/methods return constant reference like in sct_get_if

using namespace sc_core;
using namespace sc_dt;

struct Simple {
    bool a;
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
    SC_CTOR(M) {}
    
    sc_signal<bool> c;
    sc_signal<T> s;
    sc_signal<T> t;
    
    T cond_read_val() {
        if (c) return s.read();
        else return t.read(); 
    }

    const T& cond_read_ref() {
        if (c) return s.read();
        else return t.read(); 
    }    
};

// Record (structure/class) non-module tests
template <unsigned N>
struct A : public sc_module {
    using T = Simple;

    sc_in<bool> clk;
    sc_signal<bool> nrst;
    
    sc_signal<bool> c;
    sc_signal<T> s;
    sc_signal<T> t;
    
    M<T>   SC_NAMED(m);
    
    SC_CTOR(A) : always_ready(false) {
        SC_METHOD(sig_ref_meth1); 
        sensitive << c << s << t << c1;

        SC_CTHREAD(sig_ref_thrd, clk.pos()); 
        async_reset_signal_is(nrst, 0);

        SC_METHOD(sig_ref_meth2); 
        sensitive << m.c << m.s << m.t;
    }
    
    const bool always_ready;
    bool chan_sync;
    
    void before_end_of_elaboration() override {
        chan_sync = false;
    }
    
    T cond_read_val() {
        if (c) return s.read();
        else return t.read(); 
    }    
    
    sc_signal<bool> c1;
    const T& cond_read_ref() {
        c1 = !c1;
        if (c) return s.read();
        else return t.read(); 
    }
    
    sc_signal<bool> c2;
    const T& noncond_read_ref() {
        c2 = !c2;
        return s.read();
    }

    sc_signal<bool> c3;
    const T& constcond_read_ref() {
        c3 = !c3;
        if (always_ready) {
            if (chan_sync) return s.read();
            else return t.read(); 
        } else {
            if (!chan_sync) return s.read();
            else return t.read(); 
        }
    }

    sc_signal<T> t1{"t1"};
    void sig_ref_meth1() 
    {
        T l;
        l = cond_read_val();      // Correct
        l = cond_read_ref();      // Incorrect as reference cannot be inserted
        t1 = l;
    }    
    
    sc_signal<T> t2{"t2"};
    void sig_ref_thrd() 
    {
        wait();
        while(true) {
            const T& l = constcond_read_ref(); // Correct
            t2 = l;
            const T& ll = noncond_read_ref(); 
            wait();

            t2 = ll;                   // Incorrect usage after wait(); 
        }
    }    
    
    sc_signal<T> t3{"t3"};
    void sig_ref_meth2() 
    {
        T l;
        l = m.cond_read_val();      // Correct
        l = m.cond_read_ref();      // Incorrect as reference cannot be inserted
        t3 = l;
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

