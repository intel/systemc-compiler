/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include "sct_assert.h"
#include "sct_comb_signal.h"
#include "systemc.h"
#include <iostream>

using namespace sc_core;
using namespace sct;


// Use/definition analysis and CPA evaluation for record, record array, 
// record with array and record with inner record

struct Simple {
    bool a;
    int b = 0x42;
    
    bool operator == (const Simple& other) {
        return (a == other.a && b == other.b);
    }
    
    void set(int par) {
        b = par;
    }
};

inline ::std::ostream& operator << (::std::ostream& os, 
                                    const Simple& s) {
    os << s.a << s.b;
    return os;
}

struct Par {
    int a; 
    Par() {a = 42;}
    Par(int a_) {
        a = a_;
    }
    bool operator == (const Par& other) {
        return (a == other.a);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, 
                                    const Par& s) {
    os << s.a;
    return os;
}


// ---------------------------------------------------------------------------

struct Arr {
    sc_uint<12> b =0x42;
    sc_uint<20> c[3] = {1,2,3};
    
    bool operator == (const Arr& other) {
        if (b != other.b) return false;
        for (int i = 0; i < 3; ++i) if (c[i] != other.c[i]) return false;
        return true;
    }
};

inline ::std::ostream& operator << (::std::ostream& os, 
                                    const Arr& s) {
    os << s.b << s.c[0] << s.c[1] << s.c[2];
    return os;
}

// ---------------------------------------------------------------------------

struct Inn {
    sc_uint<12> b =0x42;
    Simple s;
    
    bool operator == (const Inn& other) {
        return (b == other.b && s == other.s);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, 
                                    const Inn& s) {
    os << s.b << s.s;
    return os;
}

namespace sc_core {
void sc_trace( sc_trace_file* , const Simple& , const std::string& ) {}
void sc_trace( sc_trace_file* , const Par& , const std::string& ) {}
void sc_trace( sc_trace_file* , const Arr& , const std::string& ) {}
void sc_trace( sc_trace_file* , const Inn& , const std::string& ) {}
}


// ---------------------------------------------------------------------------

class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    sc_signal<unsigned> s{"s"};
    
    sc_signal<Simple>   sim{"sim"};
    sc_signal<Arr>      aim{"aim"};
    sc_signal<Inn>      iim{"iim"};
    sc_signal<Par>      pim{"pim"};
    
    sct_comb_signal<Simple>   csim{"csim"};

    SC_CTOR(A) {
        cout << "use_def test\n";
        
        SC_METHOD(testDefinedMeth); sensitive << s;
        SC_CTHREAD(testDefined, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(testMeth); sensitive << s;
        SC_METHOD(clearMeth); sensitive << sim << s;
        SC_METHOD(clearArrMeth); sensitive << aim << s;
        //SC_METHOD(clearInnMeth); sensitive << iim << s << sim;   // See #127
        SC_METHOD(setMeth); sensitive << sim << s;
        SC_METHOD(setArrMeth); sensitive << sim << s;
        SC_METHOD(recRefMeth); sensitive << sim << s;
        SC_METHOD(combSigMeth); sensitive << csim << s;
        
        SC_METHOD(methReg); sensitive << sim << s;
        SC_CTHREAD(threadReg, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    struct CoreRsp {
        sct_uint<1>     oper;  // 0 - read, 1 - write
        sct_uint<1>     error;
        CoreRsp(const sct_uint<2>& bts) { unpack(bts); }
        void unpack(const sct_uint<2>& bts) {
            (oper, error) = bts;
        }
        
        bool operator == (const CoreRsp& other) {
            return (oper == other.oper && error == other.error);
        }
    };

    void testDefinedMeth() {
        Simple ss;
        sct_assert_defined(ss.a);
        sct_assert_defined(ss.b);
        if (ss.b) {
            ss.b = s.read()+1;
        }
        Par pp(s.read());
        sct_assert_defined(pp.a);
        if (pp.a) {
            pp.a = s.read()+2;
        }
    }
    
    /// Checking record fields are defined after ctor
    void testDefined() {
        wait();
        while (true) {
            if (s.read()) {
                CoreRsp resp(s.read());
                if (resp.error) {
                    s.write(resp.oper);
                }
            }
            wait();
        }
    }

    void testMeth() {
        Simple ss;
        sct_assert_const(ss.b == 0x42);
        ss.b = 0x43;
        sim = Simple();
        sct_assert_const(ss.b == 0x43);
    }
    
    // Clear values
    void clearMeth() {
        unsigned i = s.read();
        Simple ss;
        Simple as[3];
        
        sct_assert_const(ss.b == 0x42);
        ss = sim;  
        sct_assert_unknown(ss.b);
        sct_assert_array_defined(ss.a);
        sct_assert_array_defined(ss.b);
        sct_assert_read(sim);
        
        ss.b = 0x41;
        as[1] = ss;
        sct_assert_array_defined(as[1].a);
        sct_assert_array_defined(as[1].b);
        sct_assert_const(as[1].b == 0x41);
        as[1] = sim;
        sct_assert_unknown(as[1].b);
        
        ss.a = 1; ss.b = 0x43;
        as[2] = ss;
        sct_assert_const(as[2].a == 1);
        sct_assert_const(as[2].b == 0x43);
        as[i] = sim;
        sct_assert_unknown(as[2].a);
        sct_assert_unknown(as[2].b);
    }
    
    void clearArrMeth() {
        unsigned i = s.read();
        Arr ss;
        Arr as[3];
        
        sct_assert_const(ss.b == 0x42);
        ss = aim;  
        sct_assert_unknown(ss.b);
        
        ss.b = 0x41;
        as[1] = ss;
        sct_assert_const(as[1].b == 0x41);
        as[1] = aim;
        sct_assert_unknown(as[1].b);
        
        ss.b = 0x43; ss.c[0] = 4;
        as[2] = ss;
        sct_assert_const(as[2].b == 0x43);
        sct_assert_const(as[2].c[0] == 4);
        as[i] = aim;
        sct_assert_array_defined(as[i].b);
        sct_assert_array_defined(as[i].c[0]);
        sct_assert_array_defined(as[i].c[1]);
        sct_assert_array_defined(as[i].c[2]);
        sct_assert_unknown(as[2].b);
        sct_assert_unknown(as[2].c[0]);

        ss.c[i] = 5;
        sct_assert_unknown(ss.c[0]);
        sct_assert_unknown(ss.c[1]);
        sct_assert_unknown(ss.c[2]);
    }
    
    // Error -- Inner record is not declared 
    void clearInnMeth() {
        unsigned i = s.read();
        Simple ss;
        Inn ii;
        
        sct_assert_const(ii.b == 0x42);
        sct_assert_const(ii.s.b == 0x42);
        ii = iim;  
        sct_assert_unknown(ii.b);
        sct_assert_unknown(ii.s.b);
        
        ii.s = ss;
        sct_assert_const(ii.s.b == 0x42);
        ii.s = sim;
        sct_assert_unknown(ii.s.b);
        ss = ii.s;
        sct_assert_unknown(ss.b);
    }
    
    // Assign values
    void setMeth() {
        unsigned i = s.read();
        Simple ss;
        Simple as[3];
        
        as[0].a = 1; as[0].b = 2;
        as[1] = as[0];
        sct_assert_const(as[1].a == 1);
        sct_assert_const(as[1].b == 2);

        ss = as[1];
        sct_assert_const(ss.a == 1);
        sct_assert_const(ss.b == 2);
        
        sim = ss;
        sct_assert_unknown(sim.read());
        sct_assert_unknown(sim.read());
    }
    
    void setArrMeth() {
        unsigned i = s.read();
        Arr ss;
        Arr as[3];
        
        as[0].c[2] = 5; as[0].b = 2;
        as[1] = as[0];
        sct_assert_const(as[1].c[2] == 5);
        sct_assert_const(as[1].b == 2);

        ss = as[1];
        sct_assert_const(ss.c[2] == 5);
        sct_assert_const(ss.b == 2);
        
        aim = ss;
        sct_assert_unknown(aim.read());
        sct_assert_unknown(aim.read());
    }
    
    void recRefMeth() {
        unsigned i = s.read();
        Simple ss;

        Simple& rs = ss; 
        sct_assert_const(rs.b == 0x42);
        sct_assert_const(ss.b == 0x42);
        Simple ll1 = rs;
        Simple ll2; ll2 = rs;

        i = rs.b;
        sct_assert_read(ss.b);
        
        rs.b = 0x43;
        sct_assert_const(rs.b == 0x43);
        sct_assert_const(ss.b == 0x43);
        
        ss.b = 0x44;
        sct_assert_const(rs.b == 0x44);
        sct_assert_const(ss.b == 0x44);

        rs = sim;
        sct_assert_unknown(rs.b);
        sct_assert_unknown(ss.b);
        
        Simple rr;
        rr.a = 1; rr.b = 0x45;
        rs = rr;
        sct_assert_const(rs.a == 1);
        sct_assert_const(rs.b == 0x45);
        sct_assert_const(ss.a == 1);
        sct_assert_const(ss.b == 0x45);
    }
    
    void combSigMeth() {
        Simple ss;
        ss = csim;
        sct_assert_array_defined(ss.b);
        sct_assert_unknown(csim.read());
        sct_assert_unknown(ss.b);
        
        csim = ss;
        sct_assert_read(ss.b);
    }

    void methReg() {
        Simple ss;
        //Simple ss1{};           // prohibited
        //Simple ss2 = Simple{};  // prohibited
        //Simple ss3 = Simple();  // prohibited
        sct_assert_const(ss.b == 0x42);
        Simple tt = ss;

        Par pp1(1);
        Par pp2{2};
        Par pp = pp1;
        sct_assert_const(pp.a == 1);
        pp = pp2;
        sct_assert_const(pp.a == 2);
    }
     
    void threadReg() {
        Simple ss;
        sim = ss;
        Par pp(1);
        wait();
        
        while (true) {
            ss = sim;
            Simple tt = ss;         
            pp = pim;
            Par xx = pp;
            wait();
        }
    }
    
};
    
//sc_signal<Simple> s{"s"};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1 , SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}

