/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "systemc.h"
#include <iostream>

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

// Record (structure/class) non-module tests
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<unsigned> t{"t"};

    sct_in<sct_zero_width>  SC_NAMED(in);
    sct_out<sct_zero_width> SC_NAMED(out);
    
    SC_CTOR(A) {
        SC_METHOD(record_return1);
        sensitive << t;

        SC_METHOD(record_return2);
        sensitive << t;

        SC_METHOD(record_return3);
        sensitive << t;
        
        SC_METHOD(rec_ref_return1); sensitive << t;
        
        SC_METHOD(sig_ref_return); sensitive << t << sig << rec_sig;  // #322 fixed
    }
    
//-----------------------------------------------------------------------------
    // Function with record in return
    Simple f() {
       Simple r;
       r.b = 2;
       return r;
    }
     
    void record_return1() 
    {
        Simple s = f();
        sct_assert_defined(s.a);
        sct_assert_defined(s.b);
        
        int i = s.b + 1;
    }

    Simple g(bool val1, int val2) {
       Simple r;
       r.a = t.read();
       r.b = val1+val2;
       return r;
    }
    
    void record_return2() 
    {
        Simple s = g(true, 2);
        sct_assert_defined(s.a);
        sct_assert_defined(s.b);
        
        if (s.a) {
            s.b = 1;
        }
    }
    
    void record_return3() 
    {
        Simple s;
        if (t.read()) {
            s = f();
            sct_assert_defined(s.a);
        }
        sct_assert_defined(s.a);
        
        int i = s.b;  
    }
    
//-----------------------------------------------------------------------------
    // Function with reference to record in return
    // Function with reference to zero_width in return
    
    const Simple& func_par(Simple& par) {
        par.b = 41;
        return par;
    }
    
    Simple g_rec;
    const Simple& func_mem() {
        g_rec.b = 42;
        return g_rec;
    }
    
    sc_signal<unsigned> t2{"t2"};
    void rec_ref_return1() 
    {
        Simple s;
        Simple res = func_par(s);
        sct_assert(s.b == 41);
        t2 = res.b;
        
        res = func_mem();
        sct_assert(s.b == 42);
        t2 = res.b;
    }

    unsigned mem_var;
    const unsigned& read_var() {
        return mem_var;
    }
    
    Simple mem_rec;
    const Simple& g_mem() {
        mem_rec.b = 42;
        return mem_rec;
    }

    
    sc_signal<unsigned> sig{"sig"};
    const unsigned& read_sig() {
        return sig.read();
    }
    
    sc_signal<Simple> rec_sig{"rec_sig"};
    const Simple& read_rec_sig() {
        return rec_sig.read();
    }
    
    sc_signal<unsigned> t3{"t3"};
    void sig_ref_return() 
    {
        mem_var = t.read();
        mem_rec.b = 42;
        
        unsigned l;
        l = read_var();
        l = read_sig();
        
        Simple r;
        r = g_mem();
        r = read_rec_sig();  // BUG #322 fixed
        t3 = r.b + l;
        
        // The following is OK
        sct_zero_width z = read_zero_sig();
        const sct_zero_width& z2 = read_zero_sig2();
        t3 = z + z2;
        
        sct_zero_width z3 = read_zero_port1();
        const sct_zero_width& z4 = read_zero_port2();
        t3 = z3 + z4;
    }

//-----------------------------------------------------------------------------
    

    sc_signal<sct_zero_width> zero_sig{"zero_sig"};
    const sct_zero_width& read_zero_sig() {
        return zero_sig.read();
    }
    sct_signal<sct_zero_width> zero_sig2{"zero_sig2"};
    const sct_zero_width& read_zero_sig2() {
        return zero_sig2.read();
    }
    
    const sct_zero_width& read_zero_port1() {
        return in.read();
    }

    const sct_zero_width& read_zero_port2() {
        return out.read();
    }

    sc_signal<unsigned> t4{"t4"};
    void zero_ref_return()
    {
        sct_zero_width z = read_zero_sig();
        const sct_zero_width& z2 = read_zero_sig2();
        t4 = z + z2;
        
        sct_zero_width z3 = read_zero_port1();
        const sct_zero_width& z4 = read_zero_port2();
        t4 = z3 + z4;
    }
};

class B_top : public sc_module {
public:
    A<1> a_mod{"a_mod"};

    sct_signal<sct_zero_width>  sz;
    
    SC_CTOR(B_top) {
        a_mod.in(sz);
        a_mod.out(sz);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

