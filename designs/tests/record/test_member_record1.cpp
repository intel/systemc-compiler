/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Record with member record, which has methods and fields
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<int>      sig{"sig"};

    SC_CTOR(A) {
        SC_METHOD(mem_record1); sensitive << sig;
        
        SC_CTHREAD(mem_record2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    template <unsigned BitSize>
    struct Base {
        static const unsigned SIZE = BitSize;
        typedef sc_uint<SIZE> Bits_t;
        
        Base(const int& par) : b(par) {}
        bool a;
        Bits_t b;
        
        auto getB(unsigned offset) {
            return (b + offset);
        }
    };
    
    struct Simple : Base<8> {
        using BaseT = Base<8>;
        sc_uint<4> c;
        
        Simple(const int& par) : BaseT(par) {}
        
        auto getB(unsigned offset) {
            return BaseT::getB(offset);
        }
    };
    
    struct Complex {
        
        Base<8> b;
        Simple  s;
        
        Complex(const int& par) : b(par+1), s(par+2) {}
        
        auto getB1(unsigned offset) {
            return b.getB(offset);
        }
        auto getB2(unsigned offset) {
            return s.getB(offset);
        }
    };
    
    
    void mem_record1() 
    {
        Complex c(sig.read());
        int l = c.getB1(1) + c.getB2(2);
        
        c.b.a = true;
        c.b.b = sig.read();
        c.s.b = 42;
        c.s.c = l;
        
        l = c.b.b - c.s.c;
    }
    
    
    Complex mc{42};
    
    void mem_record2() {
        Complex cc(11);
        cc.s.a = false;
        wait();
        
        while(true) {
            mc.b.b = sig.read();
            cc.s.c = mc.b.b;
            
            int l = mc.getB1(1);
            l = cc.getB1(1);
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

