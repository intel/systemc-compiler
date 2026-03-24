/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Report warning for global variable
// Record as global and member variable to check field initialization
struct ScSimple {
    const sc_uint<2> A = 1; 
    static const unsigned B = 2;
    unsigned a = 3;
    sc_uint<10> b = 4;
    
    sc_uint<15> c;
    int d;

    ScSimple() : d(5) {};
    
    ScSimple& operator=(const ScSimple& other)
    {a = other.a; b = other.b; return *this;}
    inline friend bool operator==(const ScSimple& lhs, const ScSimple& rhs)
    {return (lhs.a == rhs.a && lhs.b == rhs.b);}
    inline friend std::ostream& operator<< (std::ostream& os, const ScSimple &obj)
    {return os;}
    inline friend void sc_trace(sc_trace_file*& f, const ScSimple& val, std::string name) 
    {}
};

int i;
const int c1 = 42;
constexpr int c2 = 43;

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
    
    ScSimple mem;       // No fields initialization for member record
    
    sc_signal<int> t0;
    void member_record() {
        ScSimple loc;
        t0 = mem.a + mem.b + mem.A + mem.B;
        t0 = loc.a + loc.b;
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

