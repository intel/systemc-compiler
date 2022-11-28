/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

template <unsigned N>
struct A : public sc_module {
    sc_uint<N> a;

    SC_CTOR(A) 
    {}
};

template <unsigned N>
struct B : public A<N>
{
    sc_in<bool>         clk{"clk"};

    B(const sc_module_name& name) : A<N>(name)
    {}
};

typedef A<2> A_;
typedef B<3> B_;

struct Top : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    A_      a{"a"};
    B_      b{"b"};
    
    SC_CTOR(Top) 
    {
        b.clk(clk);
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    sc_signal<bool> rst{"rst"};
    Top top{"top"};
    top.clk(clock_gen);
    top.rst(rst);
    sc_start();

    return 0;
}

