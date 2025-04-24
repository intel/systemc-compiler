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
    
    sc_signal<int> s;
    sc_signal<D>         sig{"sig"};
    sc_signal<Simple>    ssig{"ssig"};

    SC_CTOR(A) {
        reg.clk_nrst(clk, nrst);
        SC_METHOD(record_reg); sensitive << reg;

        SC_METHOD(record_chan1); sensitive << sig << ssig;

        SC_CTHREAD(record_chan2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(record_chan3, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(record_chan4); sensitive << s << sig;
        
        SC_METHOD(record_chan5); sensitive << s << sig;

        SC_CTHREAD(record_chan6, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(record_chan7); sensitive << s;

        SC_CTHREAD(record_chan8, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sct_register<ZeroField>  reg{"reg"};
    sc_signal<int> t0;
    void record_reg() 
    {
        reg.reset();
        ZeroField r = reg.read();
        t0 = r.b;
    }
    
    sc_signal<int> t1;
    sc_signal<D>   t2{"t2"};
    void record_chan1() 
    {
        Simple r;
        Simple t;
        r = t;
        r = ssig;
        
        D v;
        D w;
        v = sig;
        w = v;
        
        t2 = v;
        
        t1 = sig.read().c; 
        t1 = r.d + w.c;
    }
    
    sc_signal<int> t3{"t3"};
    sc_signal<D>   t4{"t4"};
    void record_chan2() {
        wait();
        while(true) {
            D x;
            x = sig;
            wait();
            t4 = x;
            t3 = x.b;  
        }
    }
    
    sc_signal<D>   t5{"t5"};
    void record_chan3() {
        D d;
        d.b = D::SIZE;
        d.c = B::SIZE;
        t5 = d;
        wait();
        while(true) {
            t5 = sig;
            wait();
        }
    }

    sc_signal<D>   t6{"t6"};
    void record_chan4() {
        D d;
        d.b = D::SIZE + D::ARR[0];
        d.c = B::SIZE + D::ARR[s.read()];
        d.c = sig.read().SIZE + d.SIZE + d.ARR[1];
        t6 = d;
    }

//-----------------------------------------------------------------------------
// Array od record with base class    
    sc_vector<sc_signal<D>> t7{"t7",2};
    //D m7[2];
    void record_chan5() {
        D d;
        D e[2];
        e[0].b = D::SIZE;
        e[1].c = B::SIZE;
        e[s.read()] = d;
        e[s.read()].c = e[s.read()].SIZE;
        //m7[s.read()].b = m7[s.read()+1].c + e[1].b;
        //m7[s.read()] = d;
        t7[s.read()] = e[s.read()];
    }

    sc_vector<sc_signal<D>> t8{"t8",2};
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
    
    sc_signal<int> t9{"t9"};
    D m8;
    D m9[2];
    sc_vector<sc_signal<D>> tt9{"tt9",2};
    B mm9[2];
    void record_chan7() {
        D d;
        D e[2];
        m8 = d;
        m8 = e[0];
        m9[s.read()] = d;   
        t9 = m9[s.read()].b;
        t9 = m9[s.read()].c;
        m9[s.read()].b = m9[s.read()+1].c;

        tt9[s.read()] = d;
        mm9[s.read()] = d;   

        t9 = d.SIZE2;
        t9 = m8.SIZE2;
        t9 = m9[s.read()].SIZE2;
    }
    
    sc_signal<D> t10{"t10"};
    D m10;
    D m11[2];
    void record_chan8() {
        D f[2];
        f[0].b = m10.SIZE1;
        f[1].c = m10.SIZE1;
        t10 = f[0];
        wait();
        while(true) {
            f[s.read()+1].b = m11[s.read()].SIZE3;
            t10 = f[s.read()];
            wait();
        }
    }

};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}

