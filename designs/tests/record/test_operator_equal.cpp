/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "systemc.h"
#include <iostream>

/// Operator== for record -- not supported yet, 
/// Support it when move to C++20, where default for operator== provided

using namespace sc_core;
using namespace sc_dt;

struct Simple {
    bool a;
    int b;

    //bool operator ==(const Simple& oth) = default; // C++20
    
    bool operator ==(const Simple& oth) {
        return (a == oth.a && b == oth.b);
    }
};

namespace std {
inline ::std::ostream& operator << (::std::ostream& os, const Simple&) 
{return os;}
}

template <unsigned N>
struct A : public sc_module {
    using T = Simple;

    sc_in<bool> clk;
    sc_signal<bool> nrst;
    
    sc_signal<unsigned> t;
    sc_signal<T> s;
    
    SC_CTOR(A) {
        SC_METHOD(equal_meth); 
        sensitive << t;

        //SC_CTHREAD(equal_thrd, clk.pos()); 
        //async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<unsigned> t1{"t1"};
    void equal_meth() 
    {
        T r1;
        T r2;
        bool b = r1 == r2;
        t1 = b;
    }


    sc_signal<unsigned> t2{"t2"};
    void equal_thrd() 
    {
        wait();
        while(true) {
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 1, SC_NS};
    A<1> top{"top"};
    top.clk(clk);
    sc_start();
    return 0;
}

