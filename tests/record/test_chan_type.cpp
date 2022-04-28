/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

using namespace sc_core;


// Record (structure/class) as channel type

struct Simple {
    bool a;
    int b;
    
    bool operator == (const Simple& other) {
        return (a == other.a && b == other.b);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, 
                                    const Simple& s) 
{
    os << s.a << s.b;
    return os;
}


class A : public sc_module {
public:
    sc_signal<bool> s{"s"};
    
    sc_signal<Simple> sim{"sim"};

    SC_CTOR(A) {
        SC_METHOD(proc1); sensitive << sim;
        //SC_METHOD(proc2); sensitive << s;
    }
    
    void proc1() {
        //Simple ss = sim.read();
        Simple ss = sim;
        //bool l = sim.read().a;
    }
    
    void proc2() {
        Simple ss;
        ss.a = 1; ss.b = 2;
        sim = ss;
    }
    
};
    
int sc_main(int argc, char *argv[]) 
{
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

