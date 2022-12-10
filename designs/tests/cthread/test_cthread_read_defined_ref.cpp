/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Read/defined analysis for references, including function parameter
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_signal<bool> s;

    SC_CTOR(A)
    {
        SC_CTHREAD(var_ref, clk.pos()); 
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(var_ref_decl); 
        sensitive << s;
        
        SC_CTHREAD(fcall_const_ref, clk.pos()); 
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(fcall_const_ref2, clk.pos()); 
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(fcall_ref_assign, clk.pos()); 
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(const_ref_noninit, clk.pos()); 
        async_reset_signal_is(nrst, 0);
        
    }

    void var_ref() 
    {
        sc_uint<3> x;
        sc_uint<3> y;
        sc_uint<3>& xref = x;
        sc_uint<3>& yref = y;
        yref = 1;
        
        sct_assert_read(x, 0);
        sct_assert_read(y, 0);
        sct_assert_defined(y);
        wait();
        
        while (true) {
            xref = 2;
            int i = x;
            sct_assert_register(x, 0);
            sct_assert_defined(x);
            
            i = yref + 1;
            sct_assert_register(y);
            sct_assert_read(y);
            
            wait();
        }
    }
    
    // Reference declaration does not lead to read of referenced variable
    void var_ref_decl() 
    {
        sc_uint<3> x;
        sc_uint<3> y;
        sc_uint<3>& xref = x;
        sct_assert_read(x, 0);
        
        sc_uint<3>& xxref = xref;
        sc_uint<3>& yref = y;
        sct_assert_read(x, 0);
        sct_assert_read(y, 0);
        
        yref = xref + 1;
        sct_assert_read(x);
        sct_assert_defined(y);
        
        xxref = yref;
        sct_assert_read(y);
        sct_assert_defined(x);
        
        xxref++;
        sct_assert_const(x == 2);
        sct_assert_const(y == 1);
    }
    
//-----------------------------------------------------------------------------

    // Constant reference parameter 
    template<class T>
    void const_ref(const T& par) {
        auto i = par;
    }

    sc_signal<sc_uint<4>> si;
    void fcall_const_ref() 
    {
        int i;
        sct_assert_register(i, false);
        sc_uint<2> k = 0;
        wait();
        
        while (1) 
        {
            const_ref(i);
            const_ref(k);
            sct_assert_register(i);
            sct_assert_register(k);

            sc_uint<2> m = 1;
            const_ref(m);
            sct_assert_register(m, false);

            sc_uint<2> n = 2;
            wait();
            
            const_ref(n);
            sct_assert_register(n);
        }
    }    
    
    void fcall_const_ref2() 
    {
        int i;
        sc_uint<2> k = 0;
        wait();
        
        while (1) 
        {
            const_ref(i + 1);
            const_ref(k || 1);
            sct_assert_register(i);
            sct_assert_register(k);

            sc_uint<2> m = 1;
            const_ref(2 & m);
            sct_assert_register(m, false);

            sc_uint<2> n = 2;
            wait();
            
            const_ref(2 << n);
            sct_assert_register(n);
        }
    }    
    
    // Assign through reference
    template<class T>
    void ref_assign(T& par) {
        par = 0;
    }
    void fcall_ref_assign() 
    {
        int i;
        sc_uint<2> k = 0;
        wait();
        
        while (1) 
        {
            ref_assign(i);
            ref_assign(k);
            const_ref(i);
            const_ref(k ? 1 : 2);
            sct_assert_defined(i);
            sct_assert_defined(k);
            sct_assert_register(i, false);
            sct_assert_register(k, false);

            sc_uint<2> m;
            ref_assign(m);
            auto j = m / 2;
            sct_assert_defined(m);
            sct_assert_register(m, false);

            wait();
        }
    }    
    
    // Constant reference initialized with non-initialized variable, 
    // warning reported
    void const_ref_noninit() 
    {
        wait();
        
        while (1) 
        {
            sc_int<3> m;
            int k;
            int p;
            int q = 1;
            const_ref(k);       // warning
            const_ref(m);
            wait();
            
            const_ref(p);       // warning
            const_ref(q);
        }
    }    
    
};

class B_top : public sc_module
{
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

