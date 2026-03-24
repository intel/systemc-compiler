/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "sct_common.h"
#include "sct_sel_type.h"
#include "systemc.h"

// Record with base class and with static constant fields used as channel payload
// used in MIF vector

struct B {
    sc_uint<4> b;

    B() = default;
};

struct D : public B {
    sc_uint<4> c;
    
    D() = default;

    bool operator ==(const D& other) {
        return (c == other.c && B::b == other.B::b);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, const D& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const D&, const std::string&) {}
}

//------------------------------------------------------------------------------

class A : public sc_module, sc_interface {
public:
    static const int C = 42;
    
    sc_in_clk clk;
    sc_signal<bool> nrst;
    
    sc_signal<int> s;

    SC_CTOR(A) {
        SC_CTHREAD(issue_325_base, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    // Issue with base record field -- index lost -- FIXED
    sc_signal<int> t0;
    void issue_325_base() 
    {
        D dr;                         // #325 issue for @dr.b, not for @dr.c -- FIXED  
        dr.b = 41;
        wait();
        
        while (true) {
            D dt;                     // #325 issue for @dt.b, not for @dt.c -- FIXED
            t0 = dr.c + dr.b;
            wait();
            t0 = dt.c + dt.b;
        }
    }
};

struct Top : public sc_module {
    sc_in_clk clk;
    sc_vector<A>   SC_NAMED(a,2);
    SC_CTOR(Top) {
        a[0].clk(clk);
        a[1].clk(clk);
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    Top a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}

