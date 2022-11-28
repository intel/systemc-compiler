/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"


// Pointer comparison and pointer to boolean conversion in conditions
class A : public sc_module 
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     rstn{"rstn"};

    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<bool>     s{"s"};
    sc_signal<bool>*    ps;
    
    int                 m;
    int                 k;
    int*                p;
    int*                p2;
    int*                p3;
    int*                q;
    sc_uint<5>*         px;

    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        p = sc_new<int>();
        p2 = sc_new<int>();
        p3 = sc_new<int>();
        q = nullptr;
        ps = new sc_signal<bool>("ps");
        px = sc_new<sc_uint<5>>();
        
        SC_CTHREAD(test_ptr_comp, clk.pos()); 
        async_reset_signal_is(rstn, 0);

        SC_CTHREAD(test_ptr_to_bool, clk.pos()); 
        async_reset_signal_is(rstn, 0);
        
        SC_CTHREAD(test_ptr_cond, clk.pos()); 
        async_reset_signal_is(rstn, 0);
    }

    // Pointer comparison to boolean conversion
    void test_ptr_comp() {
        wait();
        
        while (true) {
            bool b;
            b = p;
            b = !p;
            b = q;
            b = !q;

            b = p == nullptr;
            b = p != nullptr;

            b = p == q;
            b = p != q;
            auto x = px;
            bool c = x;

            wait();
        }
    }

    // Pointer to boolean conversion in conditions
    void test_ptr_to_bool() {
        wait();
        
        while (true) {
            unsigned i;
            if (!p2) {i = 0;}
            if (p2) {i = 1;}
            if (!p2) {i = 0;}
            sct_assert_const(i == 1);
            if (p2 != nullptr) {i = 2;}
            sct_assert_const(i == 2);

            if (q) {i = 0;}
            if (!q) {i = 3;}
            sct_assert_const(i == 3);
            if (q == nullptr) {i = 4;}
            sct_assert_const(i == 4);
            wait();
        }
    }
    
    void test_ptr_cond() {
        *p3 = 42;
        wait();
        
        while (true) {
            int i;
            i = p3 ? *p3 : 0;
            i = q ? *q : 1;
            wait();
        }
    }
};

class B_top : public sc_module 
{
public:
    sc_in<bool>      clk{"clk"};
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    B_top b_mod{"b_mod"};
    b_mod.clk(clk);
    sc_start();
    return 0;
}

