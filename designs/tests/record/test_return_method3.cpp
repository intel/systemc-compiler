/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "systemc.h"
#include <iostream>

// Check code generated for constant reference returned from function for 
// record and non-record
// Non-constant reference return not supported

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

template <unsigned N>
struct A : public sc_module {
    using T = Simple;

    sc_in<bool> clk;
    sc_signal<bool> nrst;
    
    sc_signal<bool> c;
    sc_signal<T> s;
    sc_signal<T> t;
    sc_signal<sc_uint<10>> u;
    
    SC_CTOR(A) {
        SC_METHOD(sig_ref_meth1); sensitive << u << t << s;

        SC_METHOD(sig_ref_meth2); sensitive << u << t << s;

        SC_METHOD(sig_ref_meth3); sensitive << t;

        SC_CTHREAD(sig_ref_thrd1, clk.pos()); 
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(sig_ref_thrd2, clk.pos()); 
        async_reset_signal_is(nrst, 0);
    }
    
    T read_rec_val() {
        return s.read(); 
    }    
    
    const T& read_rec_t_ref() {
        return t.read(); 
    }
    
    const T& read_rec_s_ref() {
        return s.read(); 
    }

    const sc_uint<10>& read_uint_ref() {
        return u.read(); 
    }
    
    int f(int par) { 
        return (par+1);
    }

    // Reference for record and temporary variable for non-record
    sc_signal<unsigned> t1{"t1"};
    void sig_ref_meth1() 
    {
        T r0 = read_rec_t_ref();
        const T& r1 = read_rec_t_ref();
        const sc_uint<10>& l = read_uint_ref();
        t1 = r0.b + r1.b + l;
        
        if (r1.a) {
            auto i = r1.b + 1;
            i = f(r1.b);

            const T& r2 = read_rec_s_ref();
            i = r2.a ? r1.b : r2.b;
            t1 = i;
        }
    }

    const T& const_rec_param(const T& par) {
        return par; 
    }

    sc_signal<unsigned> t2{"t2"};
    void sig_ref_meth2() 
    {
        const T& r1 = read_rec_t_ref();
        const T& r2 = r1;
        T r0 = r2;
        t2 = r0.b + r1.b;
        
        r0 = const_rec_param(r0);
        r0 = const_rec_param(r1);
    }

    // Get and put record by constant reference 
    sc_signal<T> z{"z"};
    void put_rec_ref(const T& par) {
        z = par; 
    }
    
    void sig_ref_meth3() 
    {
        const T& r = read_rec_t_ref();
        put_rec_ref(r);
    }
    
    // Return record by non-constant reference from function -- not supported    
//    T m;
//    T& read_mem_rec_ref() {
//        return m; 
//    }
//    
//    void sig_ref_meth1() 
//    {
//        T& r1 = read_mem_rec_ref();
//        T& r2 = r1;
//        T r0 = r2;
//    }
    
    
    // Check reference for record return
    sc_signal<T> t3{"t3"};
    void sig_ref_thrd1() 
    {
        auto r1 = read_rec_val();
        auto r2 = read_rec_t_ref();
        wait();
        while(true) {
            auto r3 = read_rec_val();
            auto r4 = read_rec_t_ref();
            const auto& rr = read_rec_s_ref();
            if (r4.a == rr.a) {
                r3 = rr;
                t3 = r3;
                t3 = r4;
                t3 = rr;
            }
            wait();
        }
    }

    sc_signal<unsigned> t4{"t4"};
    void sig_ref_thrd2() 
    {
        T r0;
        wait();
        while(true) {
            T r1 = read_rec_t_ref();
            const T& r2 = read_rec_s_ref();
            r0 = r2;
            wait();
            t4 = r0.b + r1.b + r2.b;       // Bad idea to use reference after @wait()
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

