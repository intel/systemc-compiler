/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

using namespace sc_core;

unsigned f() {
    return 42;
}


// Record ctor with function call in initializer list -- error reported
class Dut : public sc_module {
public:
    sc_in_clk               clk{"clk"};
    sc_signal<bool>         rst{"rst"};
    sc_signal<unsigned>     s{"s"};

    SC_CTOR(Dut) 
    {
        SC_METHOD(iniListRef); sensitive << s;
    }

//------------------------------------------------------------------------------    
struct A {
    sc_uint<4> a;
    sc_uint<8> b;
    
    typedef sc_uint<12> Bits_t;
    
    Bits_t toBits() const {
        Bits_t bits = (a, b);
        return bits;
    }
};

struct B {
    sc_uint<8> c;
    sc_uint<12> d;

    typedef sc_uint<20> Bits_t;
    
    Bits_t toBits() const {
        Bits_t bits = (c, d);
        return bits;
    }
};

struct Rec_t {
    // Sleek header bits
    A::Bits_t   aBits;// = this->m();
    // TL request bits
    B::Bits_t   bBits; // = f();

    unsigned m() {
        return 42;
    }
    
    Rec_t() = default;
    Rec_t(const Rec_t& val) = default;
    
    // Function call prohibited in initializer list -- error reported
    Rec_t(const A& par) : aBits(par.toBits()), bBits(0) 
    {}
    // Function call prohibited in initializer list -- error reported
    Rec_t(const A& par1, const B& par2) : aBits(par1.toBits()), bBits(par2.toBits()) 
    {}
    Rec_t(const A::Bits_t& par) : aBits(par)
    {}

    bool operator== (const Rec_t& other) const = default;

    friend std::ostream& operator<< (std::ostream& os, const Rec_t &obj) {
        os << "Rec_t{...}";
        return os;
    }

    friend void sc_trace(sc_trace_file* tf, const Rec_t& obj, const std::string& name) 
    {}
};
//------------------------------------------------------------------------------    

    sc_signal<int> t1;
    void iniListRef() {
        A a;
        B b;
        a.a = s.read();
        b.c = s.read()+1;
        //Rec_t rec(a);       // Error
        Rec_t rec(a, b);    // Error
        //Rec_t rec(s.read());
        //Rec_t rec;
        //t1 = rec.aBits + rec.bBits;
    }


};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    Dut dut{"dut"};
    dut.clk(clk);
    sc_start();
    return 0;
}
 
