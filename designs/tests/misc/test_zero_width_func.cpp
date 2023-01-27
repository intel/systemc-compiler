/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include "sct_assert.h"
#include "systemc.h"

using namespace sct;

// SC type zero width in function parameters, function return value, and references

// Some fields are zero width
struct Some {
    sct_uint<0> m;
    sct_uint<0> arr[2];
    unsigned mu;
    Some() = default;
    Some(unsigned par) : mu(par) {}
    bool operator == (const Some& oth) {
        return (m == oth.m && mu == oth.mu && 
                arr[0] == oth.arr[0] && arr[1] == oth.arr[1]);
    }
};
inline ::std::ostream& operator << (::std::ostream& os, const Some& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const Some&, const std::string&) {}
}

// All fields are zero width
struct All {
    sct_uint<0> m;
    sct_uint<0> arr[2];
    All() = default;
    All(unsigned par) {m = par;}
    bool operator == (const All& oth) {
        return (m == oth.m && 
                arr[0] == oth.arr[0] && arr[1] == oth.arr[1]);
    }
};
inline ::std::ostream& operator << (::std::ostream& os, const All& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const All&, const std::string&) {}
}


struct Rec {
    int m;
    bool operator == (const Rec& oth) { return (m == oth.m); }
};
inline ::std::ostream& operator << (::std::ostream& os, const Rec& s) {
    return os;
}
namespace sc_core {
void sc_trace(sc_trace_file* , const Rec&, const std::string&) {}
}

// -----------------------------------------------------------------------------    

template <unsigned N>
struct MyModule : public sc_module 
{
    sc_in_clk               clk{"clk"};
    sc_signal<bool>         nrst{"nrst"};
    sc_signal<sc_uint<8>>   s;
    sc_signal<sc_uint<8>>   t;

    SC_HAS_PROCESS(MyModule);
    MyModule(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(zeroTypeProc); sensitive << s;
        SC_CTHREAD(zeroTypeThrd, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(recAssignProc); sensitive << s;
        SC_METHOD(recRefProc); sensitive << s << rs; 
        SC_METHOD(recRefValue); sensitive << s << rs; 

        SC_METHOD(recConstRefProc); sensitive << s << rs; 
        
        SC_METHOD(valParamProc); sensitive << s;
        SC_METHOD(refParamProc); sensitive << s;
        SC_METHOD(locRefProc); sensitive << s;
        SC_METHOD(someRefParamProc); sensitive << s;
        SC_METHOD(allRefParamProc); sensitive << s;
        SC_METHOD(recArrProc); sensitive << s;
    }   
    
    template<class T>
    struct TRec {
        T m;
    };
    
    sct_uint<0> f() {
        int i = 42;
        t = i;
        return 0;
    }
    TRec<sct_uint<0>> g() {
        int i = 43;
        TRec<sct_uint<0>> l;
        return l;
    }
    void zeroTypeProc() {
        f();
        g();
        sct_uint<0> ll;
        ll = 1;
    }
    
    void zeroTypeThrd() {
        sct_uint<0> lk;
        sct_uint<0> lr = lk;
        unsigned j = lk + 1;
        wait();
        
        while (true) {
            sct_uint<0> ll;
            sct_uint<0> ln = 2;
            int i = lr + ll;
            wait();
            i = lr + ln;
        }
    }
    
// -----------------------------------------------------------------------------    
    
    
    // Assignment to record array at unknown index
    void recAssignProc() {
        unsigned j = s.read();
        Rec lr;
        Rec larr[3];
        larr[0].m = 1; larr[1].m = 2; larr[2].m = 3;
        larr[j] = lr;
        sct_assert_unknown(larr[0].m);
        sct_assert_unknown(larr[1].m);
        sct_assert_unknown(larr[2].m);
    }
    
    
    // Array reference issue
    template <class T>
    void refArr(T& par) {
        int i = par.m;
        T v = par;          
    }
    
    sc_signal<Rec> rs{"rs"};
    void recRefProc() {
        unsigned j = s.read();
        Rec larr[2];
        Rec& r = larr[j];
        Rec& rr = r;
        Rec l; 
        l = r;
        r = l;
        
        r.m = j;
        j = r.m;
        j = r.m + l.m * r.m;
        
        rs = r;
        rs.write(r);
        r = rs;
        r = rs.read();
        
        Rec& rrr = larr[j+1];
        r = r;
        rrr = rr;

        refArr(larr[j]);
        refArr(r);
        refArr(rr);
    }
    
    void recRefValue() {
        unsigned j = s.read();
        Rec larr[2];
        Rec& r = larr[j];
        Rec l; 
        l.m = 42;
        sct_assert_const(l.m == 42);
        l = r;
        sct_assert_unknown(l.m);
    }
    
// -----------------------------------------------------------------------------    
    
    template <class T>
    void refConstArr(const T& par) {
        int i = par.m + 1;
        T v = par;          
    }
    
    sc_signal<Rec> rn{"rn"};
    void recConstRefProc() {
        unsigned j = s.read();
        Rec larr[2];
        const Rec& r = larr[j];
        Rec l; 
        l = r;
        
        j = r.m;
        j = r.m + l.m * r.m;
        
        rs = r;
        rs.write(r);

        refConstArr(larr[j]);
        refConstArr(r);
    }
    
    
// -----------------------------------------------------------------------------    
    
    void valParam(int par) {
        int l = par;
        par = 1;
    }

    void valParamZero(sct_uint<N> par) {
        int i = 42 + par;
        sct_uint<N> l = par;
        par = 1;
    }
    
    sct_uint<N> returnZero() {
        sct_uint<N> l = 1;
        return l;
    }
    
    sct_uint<N> m;
    sct_uint<N> marr[3];
    void valParamProc() {
        unsigned k = s.read();
        
        int a = 11;
    	sct_uint<N> b;
        sct_uint<N> barr[3];
        
        valParam(a);
        valParam(b);
        valParamZero(b);
        valParamZero(barr[1]);
        valParamZero(marr[k]);
        valParamZero(1);
        valParamZero(a);
        
        returnZero();
        b = returnZero();
        k = returnZero();
        sc_uint<3> j; j = returnZero();
    }
    
// -----------------------------------------------------------------------------    
    
    void refParamZero(sct_uint<N>& par) {
        int i = 42 + par;
        sct_uint<N> l = par;
        par = 1;
    }
    
    void constRefParamZero(const sct_uint<N>& par) {
        int j = 42 + par;
        sct_uint<N> l = par;
    }

    sct_uint<N> mm;
    sct_uint<N> mma[3];
    sct_uint<N>& rm = mm;
    void refParamProc() {
        unsigned k = s.read();
        
        int a = 11;
    	sct_uint<N> b;
        sct_uint<N> barr[3];
        
        sct_uint<N>& rb = b;
        sct_uint<N>& rl = mm;
        a = 42 + rl;
        a = 42 + rm;
        a = 42 + rb;
        
        refParamZero(b);
        refParamZero(barr[k+1]);
        refParamZero(mm);

        constRefParamZero(b);
        constRefParamZero(marr[k]);
    }
    
// -----------------------------------------------------------------------------    
    
    sct_uint<N> mp;
    sct_uint<N> mpa[3];
    void locRefProc() {
        unsigned k = s.read();
        
    	sct_uint<N> b;
        sct_uint<N>& br = b;
        sct_uint<N> barr[3];
        sct_uint<N>& brr = barr[2];
        
        int l = br;
        l = brr + 1;
        br = l;
        
        sct_uint<N>& brp = barr[k];
        l = brp + 2;
        brp = 1;
        
    }
        
// -----------------------------------------------------------------------------    
    
    template<class T1, class T2>
    void recParam(T1 par1, T2& par2) {
        int k = par1.m + par2.m;
        T1 l = par2;
        T2& ll = par1;
        par1.arr[k] = 1;
        ll.m = 2;
    }
    
    template<class T>
    T recReturn(T par) {
        T l = par;
        return l;
    }
        
    Some mr;
    void someRefParamProc() {
        unsigned j = s.read();
        Some lr;
        Some larr[3];
        recParam(lr, lr);
        recParam(mr, larr[j]);
        
        recReturn(lr);
        recReturn(mr);
        mr = recReturn(lr);        
        larr[2] = recReturn(larr[1]);
        larr[j] = recReturn(mr);
    }
    
    All ma;
    void allRefParamProc() {
        unsigned j = s.read();
        All lr;
        All larr[3];
        recParam(lr, lr);
        recParam(ma, larr[j]);
        
        lr = recReturn(ma);
        larr[j] = recReturn(larr[j+1]);
    }
    
    // Array of record with array
    Some sarr[3];
    All aarr[3];
    void recArrProc() {
        unsigned j = s.read();
        Some lsarr[3];
        All  laarr[3];
        
        recParam(sarr[1], lsarr[j]);
        recParam(aarr[j+1], laarr[0]);
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk{"clk", 1, SC_NS};
    MyModule<0> top_mod{"top_mod"};
    top_mod.clk(clk);
    
    sc_start();

    return 0;
}


