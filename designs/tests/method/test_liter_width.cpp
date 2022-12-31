/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <sct_assert.h>

// Checking literas stored in state with proper width (width of the lvalue)
template<class T, unsigned N>
struct A : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(checkWidthProc); sensitive << s;
        
        SC_CTHREAD(concatProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<T> s{"s"};
    
    sc_uint<3> k;
    void checkWidthProc() 
    {
        if (s.read())  {
          k = 1;
        } else {
          k = sc_uint<3>(1);
        } 
        sct_assert_const(k == 1);

        if (s.read())  {
          k = sc_uint<23>(1);
        } else {
          k = sc_uint<2>(1);
        } 
        sct_assert_const(k == 1);
    }
    
    sc_uint<3> i;
    void concatProc() {
        sc_uint<3> j = 1;
        i = 1;   
        wait();
        
        while (true) {
            sc_uint<31> t = (j, i);
            sct_assert_const(t == 9);
            wait();
        }
    }
};

int sc_main(int argc, char* argv[])
{
    using T = sc_uint<16>;
    static const unsigned N = 16;
    
    A<T, N> a{"a"};
    sc_clock clk{"clk", 1, SC_NS};
    a.clk(clk);
    
    sc_start();
    return 0;
}

