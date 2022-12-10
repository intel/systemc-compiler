/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Functions and arrays in IF condition
struct C : public sc_module 
{
    SC_CTOR(C) {
    }
    
    static const unsigned BLOCK_NUM = 3;
    sc_signal<bool>         block_access[BLOCK_NUM];
};

class A : public C {
public:
    
    SC_HAS_PROCESS(A);

    sc_signal<bool> dummy{"dummy"};
    sc_signal<int> s{"s"};

    A(const sc_module_name& name) : C(name) {
        SC_METHOD(func_in_if1); sensitive << dummy;
        SC_METHOD(func_in_if2); sensitive << dummy;

        SC_METHOD(func_in_if_const1); sensitive << dummy;
        SC_METHOD(func_in_if_const2); sensitive << dummy;
        SC_METHOD(func_in_if_const3); sensitive << dummy;
        SC_METHOD(func_in_if_const4); sensitive << dummy;

        SC_METHOD(chan_array_in_if1); sensitive << dummy << ms_pwrin_nenable[0];
        SC_METHOD(chan_array_in_if2); sensitive << dummy << block_access[0]
                                      << ms_pwrin_nenable[0] << sleep_idle_cntr[0];        
        SC_METHOD(mem_array_in_if); sensitive << s;
        SC_METHOD(loc_array_in_if); sensitive << s;
        SC_METHOD(decl_array_in_if1); sensitive << s;
        SC_METHOD(decl_array_in_if2); sensitive << s;
    }
    
    static const unsigned BLOCK_NUM = 3;
    static const unsigned SLEEP_COUNTER_WIDTH = 2;
    static const unsigned SLEEP_COUNTER_MAX = (1<<SLEEP_COUNTER_WIDTH)-1;

    sc_signal<sc_uint<SLEEP_COUNTER_WIDTH> > sleep_idle_cntr[BLOCK_NUM];
    sc_signal<bool>         ms_pwrin_nenable[BLOCK_NUM];
    
    bool f() {
        bool k;
        return k;
    }

    bool g(int i) {
        return i;
    }

    void func_in_if1() 
    {
        int i;
        if (f() || i) {
            i = 1;
        }
    }

    void func_in_if2() 
    {
        int i;
        if (f() || g(i)) {
            i = 1;
        }
    }

    void func_in_if_const1() 
    {
        int i;
        if (false && g(i)) {
            i = 1;
        }
    }

    void func_in_if_const2() 
    {
        int i;
        if (true || f()) {
            i = 1;
        }
    }

    void func_in_if_const3() 
    {
        int i;
        if (f() || true) {
            i = 1;
        }
    }

    void func_in_if_const4() 
    {
        int i;
        if (false || g(i)) {
            i = 1;
        }
    }
    
    void chan_array_in_if1() 
    {
        int i;
        if (ms_pwrin_nenable[i] || i) {
            i = 1;
        }
    }
    
    // Access channel array in base module in IF condition
    
    // BUG in real design -- fixed
    void chan_array_in_if2() 
    {
       for (int i = 0; i < BLOCK_NUM; i++) {
            if (this->block_access[i] || ms_pwrin_nenable[i]) {
                sleep_idle_cntr[i] = 0;
            }    
        }
    }
    
    sc_uint<34> arr[5];
    bool arr2[4][3];
    void mem_array_in_if() 
    {
        int k = 0;
        for (int i = 0; i < 4; i++) {
            if (arr[i] == s.read() && arr2[i][k]) {
                k++;
            }    
        }
    }
    
    void loc_array_in_if() 
    {
        sc_bigint<10> larr[3] ={1, 2, 3};
        unsigned larr2[1][4];
        
        if (larr[s.read()] == larr2[s.read()][1]) {
            int k = 1;
        }    
    }
    
    void decl_array_in_if1() 
    {
        if (s.read()) {
            sc_int<10> larr[3] = {1, 2, 3};
            int larr2[3][3];

            larr2[s.read()][s.read()] = larr[1];
            
            if (s.read() == 3) {
                larr[0] = 0;
            }
        }    
    }
    
    void decl_array_in_if2() 
    {
        int a = 0;
        if (s.read()) {
            sc_int<10> ll[3];
            ll[1] = 1;
            a += ll[s.read()];
            
            if (s.read() == 1) {
                sc_int<10> ll[3];
                
                ll[1] = 2;
                a += ll[s.read()];
            }

            a += ll[s.read()+1];
        }    
    }

 };

class B_top : public sc_module {
public:
    
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

