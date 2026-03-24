/******************************************************************************
* Copyright (c) 2024, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// sc_vector of MIF to generate array of the same SV modules

struct SinCosTuple 
{
    int sin = 1;
    int cos = 2;

    bool operator == (const SinCosTuple& other) {
        return (sin == other.sin && cos == other.cos);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, const SinCosTuple& s) 
{
    return os;
}

namespace sc_core {
void sc_trace( sc_trace_file*, const SinCosTuple&, const std::string& ) {}
}

//------------------------------------------------------------------------------

struct Simple {
    bool a;
    int b;
    
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

//------------------------------------------------------------------------------

struct ArrRec {
    int i;
    int a[2];
    
    bool operator == (const ArrRec& other) {
        return (a[0] == other.a[0] && a[1] == other.a[1] && i == other.i);
    }
};

inline ::std::ostream& operator << (::std::ostream& os, 
                                    const ArrRec& s) 
{
    return os;
}

namespace sc_core {
void sc_trace( sc_trace_file* , const ArrRec& , const std::string& ) {
    
}
}

//------------------------------------------------------------------------------

template <unsigned N>
struct Producer : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_in<unsigned>         in{"in"};
    sc_signal<unsigned>     si{"si"};
    sc_signal<sc_uint<N>>   s{"s"};
    sc_signal<bool>         b{"b"};

    SC_HAS_PROCESS(Producer);
    
    const int L1;
    
    explicit Producer(sc_module_name, int val = 1) : L1(val)
    {
        SC_CTHREAD(operProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(threadResetProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(recProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(recArrProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(wholeRecProc, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(wholeRecArrProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(wholeRecTmpProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(recWithArrProc, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_METHOD(methProc);
        sensitive << s << in;
    }
    
    int getIntReg() {
        int res = s.read();
        wait();
        return res;
    }
    
    sc_signal<int> sarr[2];
    sc_signal<int> sarr_d[2];
    sc_signal<sc_uint<N>>   s_d;
    void operProc() {
        int k = 42;
        SCT_ASSERT_THREAD(s.read(), (1), s_d.read(), clk.pos());
        
        for (int i = 0; i < 2; i++) {
            SCT_ASSERT_LOOP(sarr[i], (1), sarr_d[i], clk.pos(), i);
        }
        
        wait();
        SCT_ASSERT_THREAD(s.read(), (2), s_d.read(), clk.pos());
        
        while (true) {
            s = s.read() ? k : k + 1;
            s = getIntReg();
            sct_assert(k != s.read());
            
            for (int i = 0; i != 2; ++i) { s = i; }
            for (int i = 0; i != 2; ++i) {
                s = i; wait();
            }
            wait();
        }
    }
    
    SCT_ASSERT(s.read(), (1,2), s_d.read(), clk.pos());
    
    sc_signal<sc_uint<N>>   s1;
    int m;
    void threadProc() {
        int l1 = 42;
        m = 42;
        wait();
        
        while (true) {
            int l3 = 42;
            int ll[2] = {42,43};
            sc_uint<42> uu[2];
            int ii[2];
            int l2 = s.read();
            s1 = s.read() + si.read() + l1 + l2 + m;
            wait();
            
            s1 = l1 + l3;
            wait(2);
            s1 = ll[0] + uu[0] + ii[0];
        }
    }
    
    sc_signal<sc_uint<N>>   s2;
    void threadResetProc() {
        int l4 = 42;
        int ll[2] = {42,43};
        sc_uint<42> uu[2];
        int ii[2];
        wait();
        
        while (true) {
            s2 = l4 + ll[0] + uu[0] + ii[0];
            wait();
        }
    }
    
    
    sc_signal<sc_uint<N>>   s3;
    SinCosTuple rm;
    void recProc() {
        SinCosTuple r1; r1.sin = 42;
        SinCosTuple r2;
        Simple ra[2];
        rm.sin = 43;
        wait();
        
        while (true) {
            SinCosTuple r3;
            r3.sin = 44;
            Simple rb[2]; rb[0].a = 45;
            s3 = r1.sin + r2.cos + r3.sin;
            s3 = ra[0].a + rb[0].b;
            wait();
            s3 = rb[s.read()].a;
        }
    }
    
    // Some array elements are used as registers -- all elements created as registers
    sc_signal<sc_uint<N>>   s4;
    Simple am[2];
    void recArrProc() {
        int ll[2] = {42,43};
        Simple al[2];
        wait();
        
        while (true) {
            Simple bl[2];
            s4 = ll[0] + am[0].a + (int)al[0].b;
            wait();
            s4 = bl[1].a;
        }
    }
    
    sc_signal<SinCosTuple> ws;
    SinCosTuple wm;
    void wholeRecProc() {
        SinCosTuple w1; w1.sin = 42;
        SinCosTuple w2;
        wait();
        
        while (true) {
            SinCosTuple w3 = w1;
            SinCosTuple w4;
            ws = w3;
            wm = w2;
            wait();
            
            ws = wm;
            ws = w4;
            w3 = ws;
        }
    }
    
    
    sc_vector<sc_signal<Simple>> vs{"vs", 2};
    Simple vm[2];
    void wholeRecArrProc() {
        Simple v1[2]; v1[0].a = 42;
        Simple v2[3][2];
        wait();
        
        while (true) {
            Simple v3[2];
            v3[0]= v2[2][1];
            v2[0][0] = v1[0];
            Simple v4[2];
            vs[s.read()] = v3[s.read()+1];
            vm[1] = v2[s.read()][1];
            wait();
            
            vs[0] = vm[0];
            vs[1] = v4[1];
            v3[1] = vs[s.read()];
        }
    }
    
    Simple getSimple() {
        Simple res;
        return res;
    }
    
    sc_signal<Simple> mt;
    sc_signal<sc_uint<N>>   s5;
    void wholeRecTmpProc() {
        Simple tt[2]; 
        Simple t1 = getSimple();
        wait();
        
        while (true) {
            tt[0] = getSimple();
            mt = Simple{};
            wait();
            mt = t1; 
            s5 = tt[0].a;
        }
    }

    // Record with array
    sc_signal<sc_uint<N>>   s6;
    void recWithArrProc() {
        ArrRec f; 
        ArrRec ff[2];
        wait();
        
        while (true) {
            ArrRec ffl[2];
            ArrRec ff2[2];
            s6 = f.i;
            s6 = f.a[s.read()];
            s6 = ff[s.read()+1].a[0];
            s6 = ffl[1].a[s.read()];
            ff2[0].a[0] = s.read();
            wait();
            s6 = ff2[0].a[s.read()];
        }
    }
    
    int mm;
    void methProc() {
        int l = 42;
        mm = in.read();
        b = mm+1;
    }
    
    void setI(unsigned val) {
        si = val;
    }

    sc_uint<N> getS() {
        return s.read();
    }

    bool getB() {
        return b.read();
    }
};

template <unsigned N>
struct Consumer : sc_module {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;

    sc_signal<unsigned>     i{"i"};
    sc_signal<sc_uint<N>>   s{"s"};
    sc_signal<bool>         b{"b"};

    SC_HAS_PROCESS(Consumer);
    
    explicit Consumer(sc_module_name) 
    {
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_METHOD(methProc);
        sensitive << s;  
    }
    
    void threadProc() {
        s = 0;
        wait();
        
        while (true) {
            s = s.read() + i.read();
            wait();
        }
    }
    
    void methProc() {
        b = s.read() % 2;
    }
};


SC_MODULE(Top) {
    sc_in<bool>         clk;
    sc_signal<bool>     rstn;

    const static unsigned N = 8;
    const static unsigned M = 2;
    const static unsigned K = 3;
    
#ifdef VEC_2D
    sc_vector< sc_vector< Producer<N> >>   pp{"pp", M};
#else                                            
    sc_vector< Producer<N> >    p{"p", M, [](const char* name, size_t i){
                                            return sc_new<Producer<N>>(name, i);}};
    sc_vector< Consumer<N> >    c{"c", M};
#endif
    
    sc_signal<unsigned>    SC_NAMED(sa);
    sc_signal<unsigned>    SC_NAMED(sb);
                                            
    SC_CTOR(Top) 
    {
#ifdef VEC_2D
        for (int i = 0; i != M; ++i) {
            pp[i].init(K);
            for (int j = 0; j != K; ++j) {
                pp[i][j].clk(clk); pp[i][j].rstn(rstn);
                pp[i][j].in(sa);
        }}
#else        
        for (int i = 0; i != M; ++i) {
            p[i].clk(clk); p[i].rstn(rstn);
            c[i].clk(clk); c[i].rstn(rstn);
        }
        p[0].in(sa);
        p[1].in(sb);
        
        SC_CTHREAD(mainProc, clk.pos());
        async_reset_signal_is(rstn, false);
#endif
    }
   
#ifndef VEC_2D
    void mainProc() {
        for(int i = 0; i != M; ++i) {
            p[i].setI(0); 
        }
        wait();
        
        while (true) {
            unsigned l;
            for(int i = 0; i != M; ++i) {
               l += p[i].getS(); 
            }

            wait();
        }
    }
#endif
};


int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);
    Top top("top");
    top.clk(clk);
    
    sc_start();

    return 0;
}
