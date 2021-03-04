/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <iostream>
#include <sct_assert.h>
#include "systemc.h"

// Constant static members, constant records
struct Simple {
    bool a;
    sc_uint<4> b;

    Simple(bool a, sc_uint<4> b) : a(a), b(b) 
    {}
};

class A : public sc_module 
{
public:
    const static sc_uint<16> B; 
    const static sc_bigint<65> BB; 
    const static sc_uint<16> BBB;
    const static unsigned C;  
    const static int ARR[3]; 
    const static sc_int<8> ARRI[3]; 
    // TODO: Uncomment me after #221
    //XXX const static Simple srec;
    
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<bool> s;

    SC_CTOR(A) 
    {
        SC_METHOD(const_cond_stmt); 
        sensitive << s;

        SC_METHOD(const_method); 
        sensitive << s;
        
        SC_METHOD(const_record_method); 
        sensitive << s;
        
        SC_CTHREAD(const_record_thread, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(const_record_thread2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    // Conditional statement with CXXConstructExpr inside, #222 -- fixed
    void const_cond_stmt() 
    {
        int j;
        j = true ? (sc_uint<4>)1 : (sc_uint<4>)2;
        CHECK(j == 1);
        j = false ? (sc_uint<4>)1 : (sc_uint<4>)2;
        CHECK(j == 2);
    }
    
    
// -------------------------------------------------------------------------    
    
    // Constant member variable and array
    static unsigned getvar(void) {
        return C;
    }

    void const_method() {
        int a = B;
        a = BB.to_int();
        CHECK(a == -43);
        a = BBB;
        CHECK(a == 43);
        a = ARR[1];
        CHECK(a == 2);
        a = ARRI[1];
        CHECK(a == -5);
        a = getvar();
        unsigned b = a + C;
        CHECK(b == 2);
     }

// ----------------------------------------------------------------------------
    
    // Constant record member and local
    const Simple mrec1{false, 4};
    const Simple mrec2{false, 5};

    void const_record_method() 
    {
        const Simple rec(true, 1);
        int j;
        j = rec.b + mrec1.b;
        CHECK(j == 5);
        
        
        j = false ? (sc_uint<4>)0 : rec.b;
        CHECK(j == 1);
        
        j = rec.a ? mrec1.b : rec.b;
        CHECK(j == 4);
    }
    
    void const_record_thread() 
    {
        const Simple rec(false, 1);
        wait();
        
        while (true) {
            int j = rec.b + mrec2.b;
            wait();
        }
    } 
    
    // Check name conflict with @rec from @const_record_thread() 
    void const_record_thread2() 
    {
        const Simple rec(true, 2);
        wait();
        
        while (true) {
            int j = rec.b + B;
            wait();
        }
    } 
    
};

const sc_uint<16> A::B = 42;
const sc_bigint<65> A::BB = -43;
const sc_uint<16> A::BBB = A::B+1;
const unsigned A::C = 1;
const int A::ARR[3] = {1,2,3};
const sc_int<8> A::ARRI[3] = {4,-5,6};
// XXX const Simple A::srec{false, 4};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 1, SC_NS};
    A top_inst{"top_inst"};
    top_inst.clk(clk);
    
    sc_start(100, SC_NS);
    return 0;
}

