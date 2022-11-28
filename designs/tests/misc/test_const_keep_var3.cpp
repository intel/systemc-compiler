/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

// Check correct constant variable, static constant in template parameter

template <
    unsigned N_,
    unsigned K_,
    bool DETECT
>
class B {
public: 
    static const unsigned N = N_;       // There was a bug N not printed -- fixed
    static const unsigned HN = N;   
    static const unsigned HNN = HN;
    
    static sc_uint<8> f() {
        sc_uint<8> res = 0;
        
        res = HN;
        res = N;
        
        return res;
    }
};

template <class CODE>
struct A : public sc_module
{
    sc_in<bool>     clk;
    sc_signal<int>  s;
    sc_signal<bool>  nrst;
    int* np = nullptr;
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(methProc); 
        sensitive << s;
    }
   
    void methProc()   
    {
        // Check local state has N/HN/HNN values
        sct_assert_const(CODE::N == 3);
        sct_assert_const(CODE::HN == 3);
        sct_assert_const(CODE::HNN == 3);

        // Check global state has N/HN/HNN values
        if (CODE::N != 3) {
            *np = 1;
        }
        if (CODE::HN != 3) {
            *np = 2;
        }
        if (CODE::HNN != 3) {
            *np = 3;
        }

        auto indx = CODE::f();
    }
};

typedef B<3, 1, true> B_t;

//-----------------------------------------------------------------------------

int sc_main(int argc, char *argv[]) 
{
    A<B_t> mod1{"mod1"};
    sc_clock clk{"clk", 1, SC_NS};
    mod1.clk(clk);
    
    sc_start();
    return 0;
}

