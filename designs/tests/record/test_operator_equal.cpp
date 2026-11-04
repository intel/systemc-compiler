/******************************************************************************
* Copyright (c) 2020-2026, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_common.h"
#include "systemc.h"
#include <iostream>

/// Test for default operator== in user defined records

using namespace sc_core;
using namespace sc_dt;

struct Simple {
    bool a;
    int b;

    bool operator ==(const Simple& oth) const = default; // C++20
    
    // bool operator ==(const Simple& oth) {
    //     return (a == oth.a && b == oth.b);
    // }
};

struct ArrRec {
    sc_uint<8> x[1];
    sc_biguint<16> y[2];
    sct_uint<32> z[2][2];

    bool operator ==(const ArrRec& oth) const = default; // C++20    
};

namespace std {
inline ::std::ostream& operator << (::std::ostream& os, const Simple&) {return os;}
inline ::std::ostream& operator << (::std::ostream& os, const ArrRec&) {return os;}
}

template <unsigned N>
struct A : public sc_module {
    using T = Simple;

    sc_in<bool> clk;
    sc_signal<bool> nrst;
    
    sc_signal<unsigned> t;
    sc_signal<T> s;
    
    SC_CTOR(A) {
        SC_METHOD(equal_meth_simple); sensitive << t << sr1;
        SC_METHOD(equal_meth_array); sensitive << t << sr2[0] << sr2[1];

        SC_CTHREAD(equal_thrd, clk.pos()); 
        async_reset_signal_is(nrst, 0);

        SC_METHOD(equal_meth_arrrec); sensitive << t << sr4 << sr5[0] << sr5[1];
    }
    
    T mr1;
    sc_signal<T> sr1;
    sc_signal<unsigned> t1{"t1"};
    void equal_meth_simple() 
    {
        bool b; 
        T r1; T r2;
        b = r1 == mr1;
        b = r2 == sr1.read();
        b = sr1.read() == r2;  
        t1 = b;

        if (mr1 == r2 || r1 == r2) {
            t1 = 1;
        }
        if (mr1 == sr1.read()) {
            t1 = 1;
        }
    }

    T mr2[2];
    sc_vector<sc_signal<T>> sr2{"sr2", 2};
    sc_signal<unsigned> t2{"t2"};
    void equal_meth_array() 
    {
        bool b; 
        int i = t.read();
        T r1[2]; 
        b = r1[i] == mr2[i];
        b = sr2[i].read() == r1[i];
        t2 = b;

        if (mr2[i+1] == sr2[i-1].read()) {
            t2 = 1;
        }
    }


    T mr3[2];
    sc_vector<sc_signal<T>> sr3{"sr3", 2};
    sc_signal<unsigned> t3{"t3"};
    void equal_thrd() 
    {
        bool b; 
        T r1; T r2;
        b = r1 == mr3[0];
        wait();

        while(true) {
            int i = t.read();
            T r3 = sr3[i];
            b = mr3[i].b == mr3[i+1].b;
            b = r3 == mr3[i] || mr3[i].b == mr3[i+1].b;
            b = r2 == sr3[0].read() || sr3[1].read() == r1;
            b = sr3[i+1].read() == mr3[i+2];
            sr3[i] = sr3[i+1].read();
            t3 = b;
            wait();
        }
    }

    //------------------------------------------------------------------------------------------

    // Test for ArrRec type
    ArrRec mr4;
    ArrRec mr5[2];
    sc_signal<ArrRec> sr4{"sr4"};
    sc_vector<sc_signal<ArrRec>> sr5{"sr5", 2};
    sc_signal<unsigned> t4{"t4"};
    void equal_meth_arrrec()
    {
        bool b;
        int i = t.read();
        int j = t.read();
        ArrRec r0;
        b = r0 == mr4;                      // Warning reported
        b = sr4.read() == r0;               // Warning reported
        b = sr5[i].read() == mr5[i+1];      // Warning reported
        t4 = b;
    }


};

int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 1, SC_NS};
    A<1> top{"top"};
    top.clk(clk);
    sc_start();
    return 0;
}

