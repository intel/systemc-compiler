/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Record (structure/class) assignment and field concatenation 
template <unsigned N>
class A : public sc_module {
public:
    sc_in<bool>             inp_a{"inp_a"};
    sc_in<sc_uint<32>>      inp_c{"inp_c"};
    sc_out<bool>            outp_b{"outp_b"};
    sc_out<sc_uint<32>>     outp_d{"outp_d"};
    sc_out<sc_uint<32>>     outp_t1{"outp_t1"};
    sc_out<sc_uint<32>>     outp_t2{"outp_t2"};
    sc_out<sc_uint<32>>     outp_t3{"outp_t3"};

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) 
    {
        SC_METHOD(record_local_var1);
        sensitive << dummy << inp_c;

        SC_METHOD(record_assign1);  
        sensitive << dummy;

        SC_METHOD(record_assign2);  
        sensitive << dummy;

        SC_METHOD(record_assign3);  
        sensitive << dummy;
    }
    
//-----------------------------------------------------------------------------
    // Function with records
    struct Simple {
        bool a;
        int b;
    };
    
    // Function with records
    struct AllSig {
        sc_uint<1> c;
        sc_uint<3> d;
    };

    struct Rec1 {
        int x;
        sc_uint<32> y;
        Rec1() : y(1) {
            x = 1;
        }
    };

    // Local variables structure/class type
    void record_local_var1()
    {
        Rec1 r;
        r.x = r.y + 2 + inp_c.read();
        outp_t2 = r.x;
        outp_t3 = r.y;
    }
    

//----------------------------------------------------------------------------l-
    // Record assignment in declaration and binary operator
     
    void record_assign1() 
    {
        Simple r;
        Simple s = r;
        r = s;
        outp_b = r.a;
        outp_d = r.b;
    }

    void record_assign2() 
    {
        Simple r;
        AllSig t;
        t.d = ((sc_uint<1>)r.a, (sc_uint<2>)r.b);
    }
        
    void record_assign3() 
    {
        AllSig t;
        t.c = 1;
        t.d = 4;
        outp_t1 = (t.c, t.d);
    }
    
};

class B_top : public sc_module {
public:
    sc_signal<bool> inp_a{"inp_a"};
    sc_signal<bool> outp_b{"outp_b"};
    sc_signal<sc_uint<32>>         inp_c{"inp_c"};
    sc_signal<sc_uint<32>>         outp_d{"outp_d"};
    sc_signal<sc_uint<32>>         outp_t1{"outp_t1"};
    sc_signal<sc_uint<32>>         outp_t2{"outp_t2"};
    sc_signal<sc_uint<32>>         outp_t3{"outp_t3"};
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.inp_a(inp_a);
        a_mod.outp_b(outp_b);
        a_mod.inp_c(inp_c);
        a_mod.outp_d(outp_d);
        a_mod.outp_t1(outp_t1);
        a_mod.outp_t2(outp_t2);
        a_mod.outp_t3(outp_t3);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

