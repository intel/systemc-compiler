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

// Record with base class and with static constant fields used as port payload

struct B {
    static const unsigned SIZE = 42;
    static const unsigned SIZE1 = 43;
    typedef sc_uint<SIZE> Bits_t;

    bool a;
    Bits_t b;

    B() = default;
    B(const int& par) : b(par) {}

    sc_uint<1> getB(unsigned offset) {
        return (sc_uint<1>)b[offset];
    }
};

struct D : public B {
    sc_uint<4> c;
    static const unsigned SIZE2 = 44;
    static const unsigned SIZE3 = 45;
    static constexpr unsigned ARR[2] = {11,12};

    
    D() = default;
    D(const int& par) : B(par) {}

    auto getB() {
        return B::getB(4);
    }
    
//    D& operator =(const D& other) {
//        B::b = other.B::b;
//        c = other.c;
//        return *this;
//    }
    
    bool operator ==(const D& other) {
        return (c == other.c && B::a == other.B::a && B::b == other.B::b);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, const D& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const D&, const std::string&) {}
}

//------------------------------------------------------------------------------

struct Simple {
    int d;
    sc_uint<4> e;

    Simple()  {}
    Simple(const int& par) : d(par) {}

    bool operator ==(const Simple& other) {
        return (d == other.d && e == other.e);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, const Simple& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const Simple&, const std::string&) {}
}

//------------------------------------------------------------------------------

struct ZeroField {
    sct::sct_uint<0> a;
    int b = 0;

    ZeroField()  {}
    ZeroField(const int& par) : b(par) {}

    bool operator ==(const ZeroField& other) {
        return (b == other.b);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, const ZeroField& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const ZeroField&, const std::string&) {}
}

//------------------------------------------------------------------------------

class A : public sc_module {
public:
    static const int C = 42;
    
    sc_in_clk clk;
    sc_signal<bool> nrst;
    
    sc_signal<int>      s;
    sc_in<D>            inp{"inp"};
    sc_out<Simple>      out{"out"};

    SC_CTOR(A) {
        SC_METHOD(record_chan1); sensitive << inp;

        SC_CTHREAD(record_chan2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(record_chan3, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(record_chan4); sensitive << s << inp;
        
        SC_METHOD(record_chan5); sensitive << s << inp;

        SC_CTHREAD(record_chan6, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> t1;
    sc_signal<D>   t2{"t2"};
    void record_chan1() 
    {
        Simple r;
        out = r;
        
        D v;
        D w;
        v = inp;
        w = v;
        
        t2 = v;
        
        t1 = inp.read().c; 
    }
    
    sc_signal<int> t3{"t3"};
    sc_out<D>   t4{"t4"};
    void record_chan2() {
        wait();
        while(true) {
            D x;
            x = inp;
            wait();
            t4 = x;
            t3 = x.b;  
        }
    }
    
    sc_out<D>   t5{"t5"};
    void record_chan3() {
        D d;
        d.b = D::SIZE;
        d.c = B::SIZE;
        t5 = d;
        wait();
        while(true) {
            t5 = inp;
            wait();
        }
    }

    sc_out<D>   t6{"t6"};
    void record_chan4() {
        D d;
        d.b = D::SIZE + D::ARR[0];
        d.c = B::SIZE + D::ARR[s.read()];
        d.c = inp.read().SIZE + d.SIZE + d.ARR[1];
        t6 = d;
    }

//-----------------------------------------------------------------------------
// Array od record with base class    
    sc_vector<sc_out<D>> t7{"t7",2};
    void record_chan5() {
        D d;
        D e[2];
        e[0].b = D::SIZE;
        e[1].c = B::SIZE;
        e[s.read()] = d;
        e[s.read()].c = e[s.read()].SIZE;
        t7[s.read()] = e[s.read()];
    }

    sc_vector<sc_out<D>> t8{"t8",2};
    void record_chan6() {
        D e[2];
        e[0].b = D::SIZE;
        e[1].c = B::SIZE;
        t8[0] = e[0];
        wait();
        while(true) {
            e[s.read()].c = e[s.read()].c + e[s.read()].SIZE;
            e[s.read()].a = e[s.read()].b == 1;
            e[s.read()+1].b = s.read();
            t8[s.read()] = e[s.read()];
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    
    sc_signal<D>            sig1{"sig1"};
    sc_signal<Simple>       sig2{"sig2"};
    sc_signal<D>            t4{"t4"};
    sc_signal<D>            t5{"t5"};
    sc_signal<D>            t6{"t6"};
    sc_vector<sc_signal<D>> t7{"t7",2};
    sc_vector<sc_signal<D>> t8{"t8",2};
    
    A a_mod{"a_mod"};
    a_mod.clk(clk);

    a_mod.inp(sig1);
    a_mod.out(sig2);
    a_mod.t4(t4);
    a_mod.t5(t5);
    a_mod.t6(t6);
    a_mod.t7(t7);
    a_mod.t8(t8);
    
    sc_start();
    return 0;
}

