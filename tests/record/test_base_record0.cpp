/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Function call in record constructor
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<int>      sig{"sig"};

    SC_CTOR(A) 
    {
        SC_METHOD(call_record1); sensitive << sig;

        SC_CTHREAD(call_record2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    template <unsigned SIZE>
    struct Simple {
        using Bits_t = sc_uint<SIZE>;
        
        Bits_t bits;
        int m;
        
        // Function call in in-place initialization NOT SUPPORTED
        //Bits_t cbits{ getVal() };
        
        Simple(int par) : bits(par)
            // Function call in initializer list NOT SUPPORTED
            // cbits( getVal() ) 
        { 
            int l = incrVal(par+m);
            auto k = l + m + par;
            incrBits(l);
            incrBits(par);
            incrBits(m);
            
            setBits(k);
        }
        
        unsigned getVal() {
            return 42;
        }
        
        template<class T>
        T incrVal(const T& par) {
            return T(par+1);
        }
        
        void incrBits(unsigned par) {
            int i = 41;
            bits += par + i;
        }
        
         void setBits(unsigned par) {
            int i = 41;
            bits = par + i + m;
            bits = incrVal(par + i + m);
        }
    };
    
    void call_record1() 
    {
        Simple<8> s(sig.read());
        s.bits = 43;
        auto k = (sc_uint<1>)s.bits[3];
    }
    
    struct Simple_ {
        using Bits_t = sc_uint<15>;
        
        Bits_t bits;
        
        Simple_(const Bits_t& bts = 0) : bits(bts)
        { 
            auto l = incrVal(bts);
        }
        
        template<class T>
        T incrVal(const T& par) {
            return T(par+1);
        }
    };    
    
    void call_record2() 
    {
        Simple_ r(12);
        wait();
        
        while (true) {
            Simple<8> t(sig.read());
            
            auto k = t.bits;
            k = r.bits * 2;
            
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

