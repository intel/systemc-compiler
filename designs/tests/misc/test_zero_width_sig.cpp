/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include "systemc.h"

using namespace sct;

// SC type zero width in signals/ports

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

// -----------------------------------------------------------------------------    

// SC type zero width in signals/ports
template <unsigned N>
struct MyModule : public sc_module 
{
    SC_HAS_PROCESS(MyModule);
    sc_signal<unsigned>     ss{"ss"};
    
    MyModule(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(sigProc); sensitive << s;
        SC_METHOD(arrProc); sensitive << s;
        for (int i = 0; i < 3; ++i) sensitive << sa[i] << sv[i];
        SC_METHOD(recCopyProc); sensitive << s << mc << mb;
        SC_METHOD(recSomeProc); sensitive << s << ss << mr;
        SC_METHOD(recAllProc); sensitive << s << ss << mr;
        SC_METHOD(recArrProc); sensitive << ss;
        for (int i = 0; i < 3; ++i) sensitive << asr[i] << vsr[i];
    }   

    sc_signal<sct_uint<N>> s{"s"};
    void sigProc() {
    	sct_uint<N> b;
        b = s;
        b = s.read();
        s = b;
        s.write(b);
        s.write(42);
        
        int l = s.read();
        l = s.read();
        l = s.read() + 1;
        l = s.read() - b;
        l = 2*(s.read() + 1);
        
        int j;
        l = j = s.read();
    }
    
// -----------------------------------------------------------------------------    
    
    sc_signal<sct_uint<N>> sa[3];
    sc_vector<sc_signal<sct_uint<N>>> sv{"sv", 3};
    void arrProc() {
        unsigned k = s.read();
        sa[0] = sv[1];
        sv[0] = sv[1].read() + sa[k].read();
        sa[k+1] = sv[k].read();
        
        int l = sv[1].read();
        l = sa[1].read();
        l = sv[k].read() + 1;
        l = sa[k+1].read() - sv[k-1].read();
        l = 2*(sv[2].read() + 1);
        
        sv[1] = l;
        sv[k] = l * sa[1].read();
        sa[k+1] = l + sv[k].read();
        
        int j;
        l = j = sa[1].read();
        l = j = sv[2].read();
        l = j = sa[k+1].read();
        l = j = sv[k+1].read();
    }
    
// -----------------------------------------------------------------------------
    sc_signal<Some> mc;
    sc_signal<All> mb;
    void recCopyProc() {
        unsigned k = s.read();
        Some lr;
        Some lt = lr;
        lr = lt;
        lr = mc.read();
        mc = lt;
        
        All ll;
        All lk = ll;
        lk = ll;
        ll = mb.read();
        mb.write(ll);
    }
    
    sc_signal<Some> mr;
    void recSomeProc() {
        unsigned k = s.read();
        k = ss.read();
        Some lr;
        
        lr = mr;
        lr = mr.read();
        mr = lr;
        mr.write(lr);
        
        int l = mr.read().m;
        l = mr.read().mu;
        l = mr.read().arr[0];
        l = mr.read().arr[k];
        l = mr.read().arr[0] + 1 + mr.read().m;
        
        int j;
        l = j = mr.read().m;
        l = j = mr.read().mu;
        l = j = mr.read().arr[k];
    }
    
    sc_signal<All> ma;
    void recAllProc() {
        unsigned k = s.read();
        k = ss.read();
        All lr;
        
        lr = ma;
        lr = ma.read();
        ma = lr;
        ma.write(lr);
        
        int l = ma.read().m;
        l = ma.read().arr[0];
        l = ma.read().arr[k];
        l = ma.read().arr[0] + 1 + ma.read().m;
        
        int j;
        l = j = ma.read().m;
        l = j = ma.read().arr[k];
    }
    
    // Array of record with array
    sc_signal<Some> asr[3];
    sc_vector<sc_signal<All>> vsr{"vsr", 3};
    void recArrProc() {
        unsigned k = ss.read();
        Some lr = asr[k];
        lr = asr[2].read();
        All ar = vsr[1].read();
        ar = vsr[k+1];
        
        int l = asr[k].read().m;
        l = asr[k].read().mu;
        l = asr[2].read().arr[1];
        l = asr[k].read().arr[1];
        l = asr[k].read().arr[k+1];
        int j = vsr[k].read().arr[k];
        j = vsr[k+1].read().m;
        j = vsr[k+1].read().m;
        
        Some lsarr[3];
        All  laarr[3];
        asr[0]   = lsarr[1];
        asr[k]   = lsarr[k];
        asr[k+1].write(lsarr[k]);
        asr[k+1].write(asr[k].read());
        laarr[0]  = vsr[1];
        laarr[0]  = vsr[k].read();
        laarr[k]  = vsr[k+1];
        vsr[k+1].write(vsr[k].read());
    }
};


int sc_main(int argc, char **argv) 
{
    MyModule<0> top_mod{"top_mod"};
    sc_start();

    return 0;
}


