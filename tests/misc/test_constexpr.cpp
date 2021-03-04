/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constexpr variables and constexpr functions

// Logarithm based on constexr
constexpr unsigned myLogFunc(unsigned n, unsigned p) {
  return (n < 2 ? p : myLogFunc(n/2, p+1));
}

template<unsigned n> constexpr unsigned myLog = myLogFunc(n, 0);

template <unsigned N>
struct A : public sc_module
{
    constexpr static unsigned M = myLog<N>;

    sc_in_clk       clk;
    sc_signal<bool>         rst;
    sc_signal<unsigned>     s;
    
    SC_CTOR(A) 
    {
        SC_METHOD(logProc);
        sensitive << s;
    }    
    
    void logProc() 
    {
        constexpr unsigned i = 42;
        auto j = myLog<i> + M;
        
        // SYN hangs up
        //unsigned k = s.read();
        //k = myLogFunc(k, 0);
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clk{"clk", 1, SC_NS};
    A<11> a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();

    return 0;
}


