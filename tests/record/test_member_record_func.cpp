/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Record with member record with function call from inner record 
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<int>      sig{"sig"};

    SC_CTOR(A) {
        SC_METHOD(mem_record1); sensitive << sig;
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
        
        Complex(const int& par) : b(par+1), s(par+2) {
            int l = par;
            int k = s.getB(l);
        }
        
        int getB1(unsigned par1, unsigned &par2) {
            auto l = par1 + par2 + b.b;
            auto k = b.getB(par2);
            return s.getB(l);
        }
    };
    
    
    void mem_record1() 
    {
        Complex c(sig.read()+1);
        unsigned k;
        int l = c.getB1(1, k);
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}

