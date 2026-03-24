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
    bool aa;
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
    int d = 0;
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

class A : public sc_module, sc_interface {
public:
    static const int C = 42;
    
    sc_in_clk clk;
    sc_signal<bool> nrst;
    
    sc_signal<int> s;
    sc_signal<D>         sig{"sig"};
    sc_signal<Simple>    ssig{"ssig"};

    SC_CTOR(A) {
        reg.clk_nrst(clk, nrst);
        ireg.clk_nrst(clk, nrst);
        rreg.clk_nrst(clk, nrst);
        ureg.clk_nrst(clk, nrst);
        sreg.clk_nrst(clk, nrst);
        iareg[0].clk_nrst(clk, nrst); iareg[1].clk_nrst(clk, nrst);
        uareg[0].clk_nrst(clk, nrst); uareg[1].clk_nrst(clk, nrst);
        vreg[0].clk_nrst(clk, nrst); vreg[1].clk_nrst(clk, nrst);
        wreg[0].clk_nrst(clk, nrst); wreg[1].clk_nrst(clk, nrst);
        
        SC_METHOD(issue_325); 
        sensitive << rreg << ireg << iareg[0] << iareg[1] << vreg[0] << vreg[1] << s;
        
        SC_THREAD(issue_325_thrd);
        sensitive << ureg << sreg << uareg[0] << uareg[1] << wreg[0] << wreg[1] << s;
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(issue_325_base, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(issue_cpa, clk.pos());
        async_reset_signal_is(nrst, 0);

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
    
    
    // Issue with record in MIF -- index lost #325 -- FIXED
    sct_register<int>               ireg{"ireg"};
    sct_register<Simple>            rreg{"rreg"};
    sc_vector<sct_register<int>>    iareg{"iareg", 2};
    sc_vector<sct_register<Simple>> vreg{"vreg", 2};
    sc_signal<Simple> wa;
    sc_signal<int> iaa;
    void issue_325() 
    {
        rreg.reset();                // #325 issue for @d and @e -- FIXED
        ireg.reset();
        iareg[0].reset(); iareg[1].reset();
        vreg[0].reset(); vreg[1].reset();
        vreg[s.read()].reset();
        
        Simple l;
        rreg.write(l);
        vreg[0].write(l);
        
        iaa = iareg[s.read()].read(); 
        wa = vreg[s.read()].read();  // Fixed with replace "const T&" with "T" in sct_register
    }

    sct_register<int>               ureg{"ureg"};
    sct_register<Simple>            sreg{"rreg"};
    sc_vector<sct_register<int>>    uareg{"uareg", 2};
    sc_vector<sct_register<Simple>> wreg{"wreg", 2};
    sc_signal<int>    uab;
    sc_signal<Simple> wb;
    void issue_325_thrd() 
    {
        ureg.reset();
        sreg.reset();
        uareg[0].reset(); uareg[1].reset();
        wreg[0].reset(); wreg[1].reset();
        wait();
        
        while (true) {
            uab = uareg[s.read()].read(); 
            wb = wreg[s.read()].read();     // Lost [s.read()] index 
            wreg[s.read()].write(wb.read()); 
            wait();
        }
    }

    
    // Issue with base record field -- index lost
    sc_signal<int> ta;
    void issue_325_base() 
    {
        Simple r;
        D dr;                         // #325 issue for @dr.b, not for @dr.c   
        wait();
        
        while (true) {
            Simple t;
            D dt;                     // #325 issue for @dt.b, not for @dt.c
            ta = r.e + dr.c + dr.b;      
            wait();
            ta = t.e + dt.c + dt.b;
        }
    }

    // Issue with CPA: @e.a becomes local comb variable instead of register
    // THis issue for base class field @a only, not for @aa
    sc_signal<bool> sb;
    sc_signal<D>    tb{"tb"};
    void issue_cpa() {
        D e[2];
        wait();
        while(true) {
            e[s.read()].a = 1;      // No issue if comment his line
            tb = e[0];
            // e[0].a = 1;          // Same problem
            // tb = e[s.read()];
            wait();
        }
    }  
    
    void issue_cpa1() {
        D e[2];
        wait();
        while(true) {
            e[s.read()].a = 1;
            sb = e[0].a;
            wait();
        }
    }
    
    void issue_cpa2() {
        D e[2];
        wait();
        while(true) {
            e[s.read()].a = 1;
            sb = e[s.read()].a;
            wait();
        }
    }
    
//-----------------------------------------------------------------------------    

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
    void record_chan5() {
        D d;
        D e[2];
        e[0].b = D::SIZE;
        e[1].c = B::SIZE;
        e[s.read()] = d;
        e[s.read()].c = e[s.read()].SIZE;
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

