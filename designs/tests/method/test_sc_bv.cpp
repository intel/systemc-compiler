/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sysc/datatypes/fx/fx.h"
#include "sysc/datatypes/fx/sc_fix.h"
#include "sct_assert.h"

using namespace sc_dt;

// Unsupported types error report
class A : public sc_module 
{
public:

    sc_in<sc_bv<11>>        a{"a"};
    sc_out<sc_bv<65>>       b{"b"};
    sc_signal<sc_bv<11>>    c{"c"};
    sc_signal<sc_bv<11>>    dr{"dr"};   // read
    sc_signal<sc_bv<11>>    dw{"dw"};   // written
    sc_signal<sc_bv<11>>    du{"du"};   // unused
    sc_signal<sc_bv<16>>    s;
    sc_signal<sc_uint<42>>    us;
    sc_signal<sc_biguint<42>> bus;
    sc_signal<sc_int<42>>     is;
    sc_signal<sc_bigint<42>>  bis;
    
    sc_vector<sc_signal<sc_bv<11>>>  v{"v", 3};
    
    SC_CTOR(A)
    {
        SC_METHOD(cast); sensitive << a << b << s << bus << v[0] << v[1] << v[2] << dr;
        
        SC_METHOD(ctors); sensitive << a << b << us << bus << is << bis;
        SC_METHOD(opers); sensitive << s << a << b;
        SC_METHOD(reduces); sensitive << s << a << b;
    }
    
    sc_bv<11> J = 42;
    const sc_bv<11> CJ = 42;
    
    void cast() {
        sc_bv<11> i;
        sc_bv<16> j;
        sc_biguint<42> bu;
        bu = bus;
        
        i = a;
        c = a;
        c.write(a);
        
        c = J;
        c = CJ;
        c.write(CJ);
        
        j = s;
        s = j;
        s.write(j);
        
        v[0] = i;
        i = v[1];
        c = v[i.to_int()];
        
        sc_bv<16>& r = j;
        r = s;
        s = r;
        
        i = dr;
        dw = i;
    }
    
    void ctors() {
        int l = 42;
        sc_bv<11> i; 
        i = a.read();
        sc_bv<11> j = a.read();
        sc_bv<12> k(true);
        sc_bv<13> n(42);
        sc_bv<13> m(l);
        sc_bv<14> p1(us.read());
        sc_bv<15> p2(bus.read());
        sc_bv<16> p3(is.read());
        sc_bv<17> p4(bis.read());
        
        sc_biguint<70> ll;
        sc_bv<70> q1 = ll;
        sc_bv<70> q2 = ll+1;
        
        i = J;
        i = CJ;
        
        i = j;
        i = us.read();
        i = bus.read();
        i = is.read();
        i = bis.read();
        n = m;
        
        b = a.read();
        s = b.read();
    }

    void opers() {
        bool lb;
        int il = s.read().to_int();
        sc_uint<8> ul;
        sc_bigint<8> bl;
        sc_bv<11> i;
        sc_bv<20> j;
        sc_bv<24> k;
        
        // Bit/range selections
        i[0] = 1;
        i[il] = 1;
        i(3,1) = 5;
        lb = bool(i[1]);
        il = i(10,3).to_uint(); 
        i = i.range(10,0); 
        i = s.read().range(11,1); 

        il = a.read().to_int();
        il = b.read().to_int64();
        
        lb = i.is_01();
        il = i.length();
        il = bl.length();
        il = ul.length();
        //il = s.read().length(); -- does not work
        
        // Bitwise
        i = i & s.read();
        j = 1 | k;
        k = (i & j) ^ s.read();
        k = ~i | ~a.read();
        
        j |= k;
        i ^= b.read();
        
        // Shifts
        i = i << 4;
        j = i >> il;
        k >>= 0;
        k <<= a.read().to_int();
    }
    
    void reduces() {
        bool lb;
        sc_bv<11> i;
        
        lb = i.and_reduce();
        lb = i.or_reduce();
        lb = s.read().xnor_reduce();
        if (a.read().nor_reduce()) {
            lb = a.read().nand_reduce();
        }
    }

};

class B_top : public sc_module 
{
public:
    sc_signal<sc_bv<11>>        a{"a"};
    sc_signal<sc_bv<65>>        b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

