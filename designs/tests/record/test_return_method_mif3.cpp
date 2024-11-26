/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "systemc.h"
#include <iostream>

// Checking multiple MIF array method calls to debug declaration of 
// local record array of MIF unknown array element

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
    
    sc_signal<int> c;
    
    T m[3];
    T read_val() {
        T r;
        m[c.read()] = r;
        r = m[c.read()];
        return r;
    }
};

// Record (structure/class) non-module tests
template <unsigned N>
struct A : public sc_module {
    using T = Simple;

    sc_signal<int>      s{"s"};
    
    sc_vector<M<T>>     SC_NAMED(mif, 2);
    
    SC_CTOR(A) {
        SC_METHOD(sig_ref_meth); 
        sensitive << s << mif[0].c << mif[1].c;
    }
    
    sc_signal<T> t1{"t1"};
    void sig_ref_meth() 
    {
        T p;
        p = mif[s.read()].read_val();
        p = mif[0].read_val();
        t1 = p;
    }
};



class Top : public sc_module {
public:
    A<1> b_mod{"b_mod"};

    SC_CTOR(Top) {
    }
};

int sc_main(int argc, char *argv[]) {
    Top top{"top"};
    sc_start();
    return 0;
}

