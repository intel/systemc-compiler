/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local record in-place initialization, see #140 
struct ScSimple {
    unsigned a;
    sc_uint<10> b;

    ScSimple(unsigned a_, sc_uint<10> b_) : a(a_), b(b_) {}
    
    ScSimple& operator=(const ScSimple& other)
    {a = other.a; b = other.b; return *this;}
    inline friend bool operator==(const ScSimple& lhs, const ScSimple& rhs)
    {return (lhs.a == rhs.a && lhs.b == rhs.b);}
    inline friend std::ostream& operator<< (std::ostream& os, const ScSimple &obj)
    {return os;}
    inline friend void sc_trace(sc_trace_file*& f, const ScSimple& val, std::string name) 
    {}
};

template <unsigned N>
class A : public sc_module {
public:
    sc_in<bool>         clk;
    sc_signal<bool>     rst;
    sc_signal<int>      s;

    SC_CTOR(A) 
    {
        SC_METHOD(member_record); 
        sensitive << s;
    }
    
    sc_signal<int> t0;
    void member_record() {
        ScSimple iloc{1,2};     // In-place initialization
        t0 = iloc.a + iloc.b;
    }
};


int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A<1> a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

