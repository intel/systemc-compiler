/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Read/defined analysis for records
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_signal<bool> s;

    SC_CTOR(A)
    {
        SC_METHOD(rec_decl); 
        sensitive << s;

        SC_METHOD(rec_assign); 
        sensitive << s;

        // Record array bracket initialization not supported yet, #171
        //SC_METHOD(rec_array_decl); 
        //sensitive << s;
    }

    // Check record initialization parameters are read
    struct Simple {
        sc_uint<3> a;
        Simple(sc_uint<3> val) : a(val) {}
    };
    
    void rec_decl() 
    {
        sc_uint<3> x = 1;
        Simple r(x);
        sct_assert_read(x);
        sct_assert_defined(r.a);
    }

    void rec_assign() 
    {
        Simple r(1);
        r.a = 2;
        sct_assert_read(r.a, 0);
        sct_assert_defined(r.a);
        
        sc_uint<3> x = r.a;
        sct_assert_read(r.a);
    }

    // Record array bracket initialization
    void rec_array_decl() 
    {
        sc_uint<3> x = 1;
        Simple arr[2] = {x, sc_uint<3>(1)};
        sct_assert_read(x);
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

