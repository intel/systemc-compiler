/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>
#include <stdint.h>
#include <iostream>
#include <string>

// Level determination tests, used to debug level from AST
using namespace sc_core;

struct BB : public sc_module {
    sc_signal<bool>* arbt_ack[1];

    BB(const sc_module_name& name) : sc_module(name) {
        arbt_ack[0] = new sc_signal<bool>("");
        
    }
};

class A : public BB {
public:
    
    SC_HAS_PROCESS(A);

    sc_in_clk clk;
    sc_signal<bool> nrst;

    sc_signal<bool> s{"s"};
    bool* p = nullptr;
    
    A(const sc_module_name& name) : BB(name) {

        
        SC_METHOD(for_loop_meth);
        sensitive << t << line_oper;
        
        SC_METHOD(pref_inrc);
        sensitive << s;
        
        SC_METHOD(const_init);
        sensitive << s;
        
        SC_METHOD(no_iter_loop);
        sensitive << s;
        
        SC_METHOD(fcall_const);
        sensitive << s;
        
        SC_METHOD(binary_oper1); sensitive << s;
        SC_METHOD(binary_oper2); sensitive << s;
        
        SC_METHOD(dead_code);
        sensitive << s;
        
        SC_METHOD(simple_for);
        sensitive << s;
        
        SC_METHOD(simple_do);
        sensitive << s;
        
        SC_METHOD(while_with_binary_oper);
        sensitive << s;
        
        SC_CTHREAD(label_loop, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(for_loop, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(comma); sensitive << s;
        
        SC_METHOD(compl_cond); sensitive << t;
        
        SC_METHOD(simple_if); sensitive << s;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    static const long int NEGL = -211;
    void pref_inrc()
    {
        sc_uint<9> x = 11;
        x++; 
        sct_assert_const(x == 12);
        ++x; 
        sct_assert_const(x == 13);
    }
    
    
    int m1 = 0;
    int getInit() {
        m1 = 0;
        return (m1+1);
    }
    int getInit2() {
        m1 = 1;
        return 1;
    }
    
    void const_init() 
    {
        const int i = getInit();
        const int j = getInit();

        const int k = getInit2();
        int z = i + j + k;
        CHECK(k == 1);
    }

    const int B = 4;
    void no_iter_loop() 
    {
        int k = 0;
        for (int i = B; i < B; i++) {
            k = 4;
        }
    }
    
    unsigned f() {
        return 0;
    }
    
    void fcall_const() {
        int k = 0;
        if (f() == 1) {
            k = 1;
            sct_assert_const(0);
        } else {
            k = 2;
        }
    }
    
    const bool T = true;
    void binary_oper1() {
        bool b = T || *p;
    }
    
    void binary_oper2() {
        int i = 0, j = 0;
        if (T || j++) { j = i; }
        sct_assert_read(i);
    }
    
    const int CTRL_NUM = 0;
    void dead_code() {
        bool b = false;
        for (int i = 0; i < CTRL_NUM; i++) {
            b = b || *p;
        }
    }
    
    
    void simple_for() {
        for (int i = 0; i < 3; i++) {
            int k = 1;
        }
    }
    
    void simple_do() {
        int i = 0;
        do {
            i++;
        } while (i < 3);
    }
    
    void while_with_binary_oper()
    { 
        bool b1 = 0, b2 = 0;
        while (b1 || b2) { 
            int k = 1;      
        }
    }
    
    sc_signal<bool> line_oper;
    sc_signal<bool>  arbt_ack_write;
    sc_signal<bool> t;
    void label_loop()
    {
        arbt_ack_write = 0;
        
        wait();
    
        while (true)
        {
            bool arbt_ack_ = 0;
        LOOP:
            for (int i = 0; i < 1; i++) {
                arbt_ack_ = arbt_ack_ || t;
            }
            arbt_ack_write = arbt_ack_ && line_oper;
            wait();
        }
    }
    
    void for_loop()
    {
        wait();
    
        while (true)
        {
            bool arbt_ack_ = 0;
            for (int i = 0; i < 1; i++) {
                arbt_ack_ = arbt_ack_ || t;
            }
            wait();
        }
    }
    
    void for_loop_meth()
    {
        bool arbt_ack_ = 0;
        for (int i = 0; i < 1; i++) {
            arbt_ack_ = arbt_ack_ || t;
        }
        arbt_ack_write = arbt_ack_ && line_oper;
    }
    
// ---------------------------------------------------------------------------
    
    int f(int i) {
        return (i+1);
    }
    
    void comma() {
        int j;
        j = f(1);
        sct_assert_const (j == 2);
              
        j = 0;
        int k = (j--, j++);
        k = (j = f(1), j++);
        sct_assert_const (j == 3);
    }
    
    void compl_cond() {
        bool b = 0;
        b = b || t;
    }

    void simple_if() {
        int k = 0;
        if (s.read()) {
            k = 1;
        }
    }
    
};


int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"b_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

