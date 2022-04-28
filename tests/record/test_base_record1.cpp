/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Template record with base template class, which has methods and fields 
class A : public sc_module {
public:
    sc_signal<int> sig{"sig"};

    SC_CTOR(A) {
        SC_METHOD(base_record1);
        sensitive << sig;
    }
    
    template <unsigned BitSize>
    struct Base {
        static const unsigned SIZE = BitSize;
        typedef sc_uint<SIZE> Bits_t;
        
        Base(const int& par) : b(par) {
        }
        bool a;
        Bits_t b;
        
        sc_uint<1> getB(unsigned offset) {
            return (sc_uint<1>)b[offset];
        }
    };
    
    template <unsigned BitSize>
    struct Simple : Base<BitSize> {
        using BaseT = Base<BitSize>;
        sc_uint<4> c;
        
        Simple(const int& par) : BaseT(par) {}
        
        auto getB() {
            return BaseT::getB(4);
        }
    };
    
    void base_record1() 
    {
        Simple<8> s(sig.read());
        int l = s.getB();
    }
};

int sc_main(int argc, char *argv[]) {
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

