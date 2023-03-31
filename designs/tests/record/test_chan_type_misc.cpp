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


// Extra features required for SS channels

struct Simple {
    bool a;
    int b = 0x42;
    
    Simple() = default;
    Simple(int par) : b(par) {a = false;}
    
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

namespace sc_core {
void sc_trace( sc_trace_file* , const Simple& , const std::string& ) {}
}

class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    sc_signal<unsigned> s{"s"};
    
    sc_in<Simple>       in{"in"};
    sc_out<Simple>      out{"out"};
    sc_signal<Simple>   sim{"sim"};
    sc_signal<Simple>   rim{"rim"};
    sc_signal<Arr>      aim{"aim"};
    
    SC_CTOR(A) {
        cout << "chan_type_misc test\n";
        
        SC_METHOD(callSignalParam); sensitive << sim;
        
        SC_METHOD(recordRefDecl); sensitive << s;
        
        SC_CTHREAD(recordDeclUseDef, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(localVarInRst, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(copyCtor); sensitive << s << sim << aim;   // OK
        SC_METHOD(copyCtorIssue); sensitive << s << sim;   
        SC_METHOD(funcReturn); sensitive << s << sim << aim; // OK 
        
        //SC_METHOD(condOper); sensitive << in << sim << s; // #309
        //SC_METHOD(condOperArr); sensitive << s;
    }
    
// ----------------------------------------------------------------------------

    //void put(Simple par) { -- OK
    void put(const Simple& par) {
        Simple a; 
        a = par;
    }
    
    void callSignalParam() {
        Simple a;
        const Simple& r1 = a;             // OK
        const Simple& r2 = sim.read();      // OK
        a = r2;
        // const Simple& r3 = Simple();     // Prohibited
        
        put( sim.read() );
    }
    
// ----------------------------------------------------------------------------

    void recordRefDecl() {
        Simple ss;
        Simple& rs = ss; 
        Simple ll1 = rs;
        Simple ll2; ll2 = rs;
    }
    
    void recordDeclUseDef() {
        int i = 42;
        wait();
        
        Simple a(i);   // Check @i is used -- OK
        wait();
        
        while (true) {
            wait();
        }
    }
    
    void localVarInRst() {
        Simple s;
        wait();
        
        s.b = 43;
        const Simple& ref = s;
        Simple a; a = ref;       // Check s.a is declared and register
        
        wait();

        while (true) {
            wait();
        }
    }
    
// ----------------------------------------------------------------------------

    void copyCtor() 
    {   
        unsigned j = s.read();
        Simple ss = sim.read();
        Simple tt = sim;
        ss = tt;
        Arr aa = aim;
        ss.b = aa.b;
        ss.b = aa.c[1];
        tt.b = aa.c[j];
    }
    
    void copyCtorIssue() 
    {   
        Simple t1 = sim;
        Simple t2(s.read());
        Simple t3 = Simple(s.read());
        Simple t4 = s.read();
        
        Simple t5;
        t5 = Simple(s.read());
        
        rim.write(t5);
        rim.write(Simple());
        //rim.write(Simple(s.read()));    // error reported, that is OK
        rim = Simple();
        //rim = Simple(s.read());       // error reported, that is OK
    }

// ----------------------------------------------------------------------------
    
    Simple returnRec() {
        Simple ll;
        return ll;
    }
    
    Simple returnSigRec() {
        return sim.read();
    }
    
    Simple returnSigRec2() {
        return sim;
    }

    void funcReturn() 
    {   
        Simple ss;
        ss = returnRec();
        ss = returnSigRec();
        ss = returnSigRec2();
        
        Simple rr = returnRec();
        rim = returnSigRec();
    }

// ----------------------------------------------------------------------------

    void condOper() 
    {
        Simple ss;
        Simple rr; Simple tt;
        ss = s.read() ? rr : tt;  // Not supported yet, see #309
        //ss = s.read() ? sim.read() : in.read();
        
        //logic sim_b[10];
        //{ss_a, ss_b} = s ? {sim_a, sim_b} : {in_a, in_b};  // SV
        //ss_b = sim_b; 
    }
    
    void condOperArr() 
    {
        Arr ss; Arr rr;
        ss = rr;
        //ss = s.read() ? rr : tt;
        //ss = s.read() ? sim.read() : in.read();
        
        //logic sim_b[10];
        //{ss_a, ss_b} = s ? {sim_a, sim_b} : {in_a, in_b};  // SV
        //ss_b = sim_b; 
        
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

