/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Record with multiple base classes, which has methods and fields
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<int>      sig{"sig"};

    SC_CTOR(A) 
    {
        SC_METHOD(base_record1); sensitive << sig;
        
        SC_CTHREAD(base_record2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(base_record3, clk.pos());
        async_reset_signal_is(nrst, 0);
        
    }
    
    struct Base0 {
        int a = 42;
        
        Base0(const int& par) : a(par) {}
    };    
    
    struct Base1 : public Base0 {
        static const unsigned SIZE = 12;
        typedef sc_uint<SIZE> Bits_t;
        
        Base1(const int& par) : Base0(par), b(par) {}
        //Bits_t b;
        int b;

        auto getB() {
            return b;
        }
        
        void setB(int par) {
            b = par + 1;
        }
    };
    
    struct Base2 {
        Base2(const int& par) : c(par) {}
        unsigned c;
        
        unsigned getC() {
            return c;
        }
        
        void setC(unsigned& par) {
            c = par - 1;
        }
    };
    
    struct Simple : public Base1, public Base2 {
        sc_uint<4> d;
        
        Simple(const int& par) : Base1(par), Base2(par) {}
        
        auto getB1() {
            return Base1::b;
        }
        
        auto getB2() {
            return Base1::getB();
        }
        
        auto getC() {
            return Base2::getC();
        }
    };
    
    void base_record1()
    {
        Simple s(sig.read());
        int l;
        l = s.getB1();
        l = s.getB2();
        s.b = l;
        l = s.b;
        
        Simple& r = s;
        r.b = s.b;
        s.d = r.d;

        s.c = sig.read();
        l = s.getC();
        l = r.getC();
        r.c = s.c;
        r.setC(r.c);
    }
    
    
    Simple ms{42};
    void base_record2()
    {
        Simple ls(43);
        wait();
        
        while (true) {
            unsigned l;
            l = ms.getB1() + ls.getB2();
            ms.setB(sig.read());
            
            wait();
            
            l = ms.c;
            ms.setB(ls.b);
            ls.setC(l);
        }
    }
    
    void base_record3()
    {
        sc_uint<8> ll;
        wait();
        
        while (true) {
            Simple lt(sig.read()*3);
            ll += lt.getB1();
            
            wait();
            
            int l = lt.getC();
            lt.setB(ll+l);
        }
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

