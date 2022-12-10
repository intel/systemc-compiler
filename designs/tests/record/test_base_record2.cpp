/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Record with base class, which has methods and fields
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<int>      sig{"sig"};

    SC_CTOR(A) 
    {
        SC_METHOD(base_record1); sensitive << sig;
        SC_METHOD(base_record2); sensitive << sig;
        
        SC_CTHREAD(base_record3, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    struct Base {
        static const unsigned SIZE = 12;
        typedef sc_uint<SIZE> Bits_t;
        
        Base(const int& par) : b(par) {}
        bool a;
        Bits_t b;
        
        sc_uint<1> getA() {
            return (sc_uint<1>)a;
        }

        auto getB_() {
            return b;
        }
        
        void setB(int par) {
            b = a ? par-1 : par + 1;
        }
    };
    
    struct Simple : Base {
        sc_uint<4> c;
        
        Simple() : Base(0) {}
        Simple(const int& par) : Base(par) {}
        
        auto getB() {
            return this->getB_();
        }
    };
    
    void base_record1() 
    {
        Simple s(sig.read());
        int l = s.getB();
        s.a = l;
        s.setB(42);
        l = s.getB() + s.b;
    }


    void base_record2() 
    {
        Simple r1;
        Simple r2(r1.c+1);
        
        r1.b = sig.read();
        
        if (r1.getB()) {
            r2.setB(r1.getB_());
        }
    }
    
    Simple m{42};
    
    void base_record3() {
        Simple t1;
        auto l = t1.getB();
        wait();
        
        while(true) {
            Simple t2;
            t2.setB(sig.read());
            t1.a = sig.read() == 3;
            t2.b = t1.getB()+m.c;
            wait();
            
            t2.a = sig.read() == 4;
            l = t1.a ? t2.getB() : m.getB_();
            wait();
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

