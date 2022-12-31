/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "sct_comb_signal.h"
#include "systemc.h"
#include <iostream>

using namespace sc_core;


// Record (structure/class) as channel type in THREAD

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
void sc_trace( sc_trace_file* , const Simple& , const std::string& ) {}
}

class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    sc_signal<unsigned> s{"s"};
    //sc_signal<sc_uint<2>> sim_a{"sim_a"};
    
    sc_in<Simple>       in{"in"};
    sc_out<Simple>      out{"out"};
    sc_signal<Simple>   sim{"sim"};
    sc_signal<Simple>   rim{"rim"};
    sc_signal<Simple>   tim{"tim"};
    sc_signal<Simple>   vim{"vim"};
    sc_signal<Simple>   wim{"wim"};
    sc_vector<sc_signal<Simple>> svec{"svec", 2};
    sc_vector<sc_signal<Simple>> rvec{"rvec", 2};
    sc_vector<sc_vector<sc_signal<Simple>>> rrvec{"rvec", 2};
    sc_vector<sc_signal<Simple>> tvec{"rvec", 2};

    SC_CTOR(A) {
        cout << "chan_type_thrd test\n";
        
        for (int i = 0; i < 2; i++) rrvec[i].init(3);
        
        //SC_METHOD(methPort); sensitive << rim;
        
        SC_CTHREAD(resetRecord, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(bodyRecord, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(threadSignal, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(threadCombSignal, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(threadPort, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(threadArr, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(threadCtor, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    // Check declaration of record/record channel
    Simple mm;
    void methPort() {
        Simple ss;
        //sim = rim;
        //sim = mm;
        mm = rim;
        //sim.write(ss);
        //ss = rim;           
        //ss = rim.read();    
    }
    
    sc_signal<int> v{"v"};
    void recordField() {
        Simple ss;
        wait();
        
        while (true) {
            v = ss.b; // error
            ss.b = v; // error
            wait();
        }
    }
    
    // Check record in reset declaration as registers
    void resetRecord() {
        Simple ss;          // reg
        wait();
        
        while (true) {
            rim = ss;
            ss = rim;
            wait();
        }
    }
    
    // Check record in body declaration as registers/comb
    void bodyRecord() {
        wait();
        
        while (true) {
            Simple ss;      // comb        
            Simple rr;      // reg
            tim = ss;
            ss = tim;
            wait();
            tim = rr;
        }
    }
    
    void threadSignal() {
        Simple ss;
        sim = ss;
        wait();
        
        while (true) {
            Simple rr;
            rr.a = 1; rr.b = 2;
            
            sim = ss;
            sim = rr;
            sim.write(ss);
            
            sim = rim;
            sim = rim.read();
            
            sim = rim;      
            sim = rim.read();
            sim.write(tim);
            sim.write(tim.read());
                    
            wait();
        }
    }
    
    sct_comb_signal<sc_uint<12>>    cim{"cim"};
    sct_comb_signal<Simple>         csim{"csim"};
    sct_comb_signal<Simple>         ctim{"ctim"};
    sct_comb_signal<Simple, 0>      crim{"crim"};
    
    void threadCombSignal() {
        Simple ss;
        cim = 42;
        csim = Simple{};
        ctim = ss;
        crim = ss;
        wait();
        
        while (true) {
            Simple rr;
            rr.a = 1; rr.b = 2;

            sc_uint<12> i = cim;
            ss = csim;      
            ss = ctim;      
            ss = crim;      // @crim is reg
            csim = ss;

            csim.write(rr);
            rr = csim;
            
            csim = crim;
            ss = crim.read();
                    
            wait();
        }
    }
    
    void threadPort() {
        Simple ss;
        out = ss;
        out.write(ss);
        wait();
        
        while (true) {
            //Simple rr = in.read(); -- not supported
            ss = in;
            ss = in.read();
            
            out = in;           
            out = ss;           
            out.write(ss);  
            ss = out;
            
            out = rim;
            out = rim.read();
            out.write(rim);
            out.write(rim.read());
            
            wait();
        }
    }
    
    void threadArr() {
        Simple ss;
        Simple tt[2];
        tt[1].a = 1; tt[1].b = 2;
        wait();
        
        while (true) {
            unsigned i = s.read();
            unsigned j = s.read();
            Simple rr[2];
            rr[1].a = 1; rr[1].b = 2;
            rrvec[0][1] = ss;
            rvec[0] = ss;
            vim = ss;
            
            ss = rr[1];
            ss = svec[j];               // OK
            tt[i] = svec[j].read();     // OK  
            rvec[i] = rr[j];            // OK
            rvec[i].write(tt[j]);       // OK
            rr[i] = svec[j];            // OK
            rr[i] = svec[j].read();     // OK
            rvec[i] = svec[j];              // OK
            rvec[i] = svec[j].read();       // OK
            rvec[i].write(svec[j]);         // OK
            rvec[i].write(svec[j].read());  // OK
            rrvec[i][j] = svec[j];
            
            ss = rvec[i];
            ss = rrvec[i][j];
            wait();
        }
    }
    
    void threadCtor() {
        Simple ss;
        //Simple rr{};      -- not allowed
        //ss = Simple{};    -- not allowed 
        tvec[0] = Simple{};
        wait();
        
        while (true) {
            unsigned i = s.read();
            //ss = Simple();      -- not allowed
            wim = Simple();       // OK
            wim = Simple{};       // OK  
            wim.write(Simple());  
            wim.write(Simple{});  
            
            tvec[i] = Simple{};
            tvec[i+1] = Simple();
            
            Simple tt = ss;       // OK
            wait();
        }
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

