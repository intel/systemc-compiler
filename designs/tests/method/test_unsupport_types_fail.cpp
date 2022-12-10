/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sysc/datatypes/fx/fx.h"
#include "sysc/datatypes/fx/sc_fix.h"
#include "sct_assert.h"

using namespace sc_dt;

// Unsupported types error report
class A : public sc_module 
{
public:
    sc_signal<sc_uint<4>> s;

    SC_CTOR(A)
    {
        SC_METHOD(read_to_int); 
        sensitive << s << a << b;
    }
    
    sc_in<sc_bv<38>>        a{"a"};
    sc_in<sc_lv<42>>        b{"b"};
    //sc_in<sc_logic>         c{"c"}; -- error reported, but leads to assert violation
    sc_out<sc_biguint<111>> d{"d"};
    
    sc_fix              m;
    sc_fixed<32, 20>    n;
    
    void read_to_int() {
        int l;
        l = m + 1;
        l = n + 1;
        l = a.read().to_int();
        l = b.read().to_int();
        //l = c.read().to_char();
    }
};

class B_top : public sc_module 
{
public:
    sc_signal<sc_bv<38>>        a{"a"};
    sc_signal<sc_lv<42>>        b{"b"};
    //sc_signal<sc_logic>         c{"c"};
    sc_signal<sc_biguint<111>>  d{"d"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        //a_mod.c(c);
        a_mod.d(d);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

