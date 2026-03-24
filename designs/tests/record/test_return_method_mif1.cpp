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
    SC_CTOR(M) {
    }
    
    T mm1;
    T read_val_1() {
        T r;
        r = mm1;
        return r;
    }

    T mm2;
    T read_val_2() {
        T r;
        r = mm2;
        return r;
    }

    T mm3;
    T read_val_3() {
        T r;
        r = mm3;
        return r;
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
        SC_CTHREAD(sig_ref_thrd1, clk.pos()); 
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(sig_ref_thrd2, clk.pos()); 
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(sig_ref_thrd3, clk.pos()); 
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> s;
    
    sc_signal<T> t4{"t4"};
    void sig_ref_thrd1() {
        T p;
        p = mif[0].read_val_1();
        wait();
        
        while (true) {
            p = mif[1].read_val_1();  
            t4 = p;
            wait();
        }
    }

    sc_signal<T> t5{"t5"};
    void sig_ref_thrd2() {
        T p;
        p = mif[1].read_val_2();
        wait();
        
        while (true) {
            p = mif[0].read_val_2();  
            t5 = p;
            wait();
        }
    }

    sc_signal<T> t6{"t6"};
    void sig_ref_thrd3() {
        T p;
        p = mif[0].read_val_3();
        wait();
        
        while (true) {
            p = mif[s.read()].read_val_3();  
            t6 = p;
            wait();
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

