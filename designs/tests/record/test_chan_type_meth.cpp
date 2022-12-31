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


// Record (structure/class) as channel type in METHOD

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
                                    const Simple& s) 
{
    os << s.a << s.b;
    return os;
}

namespace sc_core {
void sc_trace( sc_trace_file* , const Simple& , const std::string& ) {
    
}
}

// ----------------------------------------------------------------------------
struct Rec1 {
    int x;
    sc_int<2> y;       
    bool operator == (const Rec1& other) {
        return (x == other.x && y == other.y);
    }
};
inline ::std::ostream& operator << (::std::ostream& os, const Rec1& s) {
    os << s.x << s.y;
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const Rec1&, const std::string&) {}
}
// ----------------------------------------------------------------------------

class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    sc_signal<unsigned> s{"s"};
    sc_signal<sc_uint<2>> sim_a{"sim_a"};   // Check name conflict
    
    sc_in<Simple>       in{"in"};
    sc_out<Simple>      out{"out"};
    sc_signal<Simple>   sim{"sim"};
    sc_signal<Simple>   rim{"rim"};
    sc_signal<Simple>   tim{"tim"};
    sc_vector<sc_signal<Simple>> svec{"svec", 2};
    sc_vector<sc_signal<Simple>> rvec{"rvec", 2};
    
    SC_CTOR(A) {
        cout << "chan_type_meth test\n";
        SC_METHOD(record_assignment); sensitive << s;
        SC_METHOD(record_channels); sensitive << sig;
        SC_METHOD(procSignal); sensitive << sim << sim_a;
        SC_METHOD(procSigField); sensitive << s << sim << svec[0] << svec[1];
        SC_METHOD(procPort); sensitive << in << sim;
        SC_METHOD(procArr); sensitive << svec[0] << svec[1] << s;
        SC_METHOD(procCtor); sensitive << s;        
    }
    
// ----------------------------------------------------------------------------
    // For User guide
    void record_assignment() {
        Rec1 r; Rec1 p;
        r = p;
        Rec1& ref = r;
        r = p;  
    }    
    
    sc_signal<Rec1>   sig{"sig"};
    void record_channels() {
        Rec1 r;
        r = sig.read();
        r = sig;
        sig.write(r);
        sig = r;
        int x = sig.read().x;
    }  
// ----------------------------------------------------------------------------
  
    Simple mm; 
    void procSignal() 
    {
        Simple ss; 
        Simple rr;
        rr.a = 1; rr.b = sim_a.read();
        ss = rr;
        sim.write(ss);     // OK
        sim = ss;          // OK
        ss = sim.read();   // OK
        ss = sim;          // OK 
        rim = sim;              // OK 
        rim = sim.read();       // OK 
        rim.write(sim);         // OK 
        rim.write(sim.read());  // OK 
        
        //sim_a = 1;
    }

    void f(int par) {
        int l = par;
    }
    
    void procSigField() {
        unsigned j = s.read();
        bool l;
        l = sim.read().a;
        l = svec[0].read().a;
        l = svec[j].read().a;
        int i;
        i = sim.read().b;
        i = svec[1].read().b;
        i = svec[j].read().b;     // need to support
        
        f(sim.read().b);
        f(svec[1].read().b);
        f(svec[j+1].read().b);
        
        if (sim.read().a) ++i;
        if (svec[1].read().a) ++i;
        if (svec[j].read().a) ++i;
        if (sim.read().b != 1) ++i;
        if (svec[j].read().b != 2) ++i;

        //const Simple& tt = sim.read();    -- prohibited
        //l = tt.a;
    }
    
    void procPort() {
        Simple ss;
        ss = in;            // OK
        ss = in.read();     // OK
        out = ss;           // OK
        out.write(ss);      // OK
        out = in;
        out = sim;
    }
    
    void procArr() {
        unsigned i = s.read();
        unsigned j = s.read();
        Simple ss;
        Simple rr[2];
        rr[1].a = 1; rr[1].b = 2;
        ss = rr[1];
        ss = svec[j];               // OK
        ss = svec[j].read();        // OK  
        rvec[i] = rr[j];            // OK
        rvec[i].write(rr[j]);       // OK
        rr[i] = svec[j];            // OK
        rr[i] = svec[j].read();     // OK
        rvec[i] = svec[j];              // OK
        rvec[i] = svec[j].read();       // OK
        rvec[i].write(svec[j]);         // OK
        rvec[i].write(svec[j].read());  // OK
    }
    
    void procCtor() {
        Simple ss;
        //Simple rr{};      -- not allowed
        //ss = Simple{};    -- not allowed
        //ss = Simple();    -- not allowed
        tim = Simple();       // OK
        tim = Simple{};       // OK  
        tim.write(Simple());  
        tim.write(Simple{});  
        out = Simple();       // OK
        out = Simple{};       // OK
        out.write(Simple{});  // OK
        out.write(Simple());  // OK
        Simple tt = ss;       // OK
    }
    
};
    
sc_signal<Simple> s{"s"};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1 , SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    a_mod.in(s);
    a_mod.out(s);
    sc_start();
    return 0;
}

