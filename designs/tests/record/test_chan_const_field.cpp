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


// Record (structure/class) with constant and static constant fields used 
// as channel type 

const int G = 44;

namespace ns {
    const int GN = 45;
}

const unsigned W = 5;
typedef sc_uint<W> WType_t;

enum E: uint8_t {
    eVal0 = 0,
    eVal1 = 1,
    eVal2 = 2
};


// ----------------------------------------------------------------------------
struct LRec {
    int y;
    
    inline LRec(int par) : y(par) {}
    
    bool operator == (const LRec& other) {
        return (y == other.y);
    }
    
    inline friend std::ostream& operator<< (std::ostream& os, const LRec &obj) {
        return os;
    }
};

// ----------------------------------------------------------------------------
struct Rec {
    //const int C = 42;  -- error reported OK
    static const int S = 43;
    int x;
    
    //Rec& operator = (const Rec& other) {x = other.x; return *this;}
  
    int f(int par) {
        const int L = 41;
        static const int SL = 42;
        return (par+L+SL);
    }
    
    inline E getE(uint8_t par) const {
        return E(par);
    }
    
    inline LRec getL(int par) {
        LRec lrec(par);
        return lrec;
    }
    
    bool operator == (const Rec& other) {
        return (x == other.x);
    }
};
inline ::std::ostream& operator << (::std::ostream& os, const Rec& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const Rec&, const std::string&) {}
}

// ----------------------------------------------------------------------------

class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    sc_signal<int>      s{"s"};
    
    sc_signal<Rec>      rs{"rs"};
    sc_vector<sc_signal<Rec>> rv{"rv", 2};
    
    SC_CTOR(A) {
        SC_METHOD(record_channel_meth); sensitive << s << rs;
        SC_METHOD(record_channel_arr_meth); sensitive << rv[0] << rv[1] << s;
    }
    
// ----------------------------------------------------------------------------
    
    sc_signal<int> t0;
    void record_channel_meth() {
        Rec r; 
        Rec t;
        r = t;
        t0 = r.x + r.S + Rec::S;

        r = rs.read();
        t0 = r.f(r.x + G + ns::GN);
        
        r = rs;
        rs.write(r);
        rs = r;

        int x = rs.read().x;
        t0 = x + r.x;

        LRec l = r.getL(s.read());
        t0 = l.y;
        
        t0 = r.getE(s.read());
        
        WType_t i = r.x;
        t0 = i;
    }  

    sc_signal<int> t1;
    void record_channel_arr_meth() {
        Rec r = rv[0];
        if (rv[1].read().x == 1) {
            r = rv[s.read()];
        }
        Rec t(rv[1]);
    }
};
    
int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1 , SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}

