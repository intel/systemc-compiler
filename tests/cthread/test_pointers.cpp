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

// Variable and channel pointers, arrays of channel pointers 
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_in<sc_uint<4>>   in;
    sc_out<sc_uint<4>>* qout{new sc_out<sc_uint<4>>("qout")};
    sc_in<sc_uint<4>>*  pin;
    
    sc_signal<sc_uint<4>> s;

    int* q0{sc_new<int>()};
    int* q1{sc_new<int>()};
    sc_uint<4>* p0;
    sc_uint<4>* p1;
    sc_uint<4> v0;
    sc_uint<4> v1;
    
    sc_signal<sc_uint<4>>* qs0{new sc_signal<sc_uint<4>>("qs0")};
    sc_signal<sc_uint<4>>* qs1{new sc_signal<sc_uint<4>>("qs1")};
    sc_signal<sc_uint<4>> s0;
    sc_signal<sc_uint<4>> s1;
    sc_signal<sc_uint<4>>* ps0;
    sc_signal<sc_uint<4>>* ps1;
    
    static const unsigned N = 4;
    static const unsigned M = 5;
    static const unsigned K = 5;
    sc_signal<sc_uint<33>>*     parr1[N];
    sc_signal<sc_biguint<42>>*  parr2[N][M];
    sc_signal<sc_bigint<65>>*   parr3[N][M][K];

    sc_in<sc_uint<33>>*         iarr1[N];
    sc_uint<33>*                arr1[N];
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        p0 = &v0; 
        p1 = &v1;
        
        ps0 = &s0; 
        ps1 = &s1;
        
        pin = &in;
        
        for (int i = 0; i < N; i++) {
            parr1[i] = new sc_signal<sc_uint<33>>("parr1");
            iarr1[i] = new sc_in<sc_uint<33>>("iarr1");
            arr1[i] = sc_new<sc_uint<33>>();
            for (int j = 0; j < M; j++) {
                parr2[i][j] = new sc_signal<sc_biguint<42>>("parr2");
                for (int k = 0; k < K; k++) {
                    parr3[i][j][k] = new sc_signal<sc_bigint<65>>("parr3");
                }
            }
        }
        
        SC_CTHREAD(var_ptr1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(var_ptr2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(sig_ptr1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(sig_ptr2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(port_ptr, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(array_ptr1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(array_ptr2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void var_ptr_meth() {
        *p0 = 2;
        int j = *p0;
    }

    void var_ptr1() {
        *p0 = 0;        // v0 is reg
        *p1 = 1;        // v1 is comb
        wait();
        
        while (true) {
            *p1 = s.read();
            int j = *p0 + *p1;
            *p0 = *p1;
            wait();
        }
    }

    void var_ptr2() {
        *q0 = 0;        // q0 is reg
        *q1 = 1;        // q1 is comb
        wait();
        
        while (true) {
            *q1 = s.read();
            int j = *q0 + *q1;
            *q0 = *q1;
            wait();
        }
    }
    
// ----------------------------------------------------------------------------    
    
    void sig_ptr1() {
        *ps0 = 0;        // s0 is reg
                         // s1 is comb (read-only)
        wait();
        
        while (true) {
            int j = ps0->read() + (*ps1).read();
            *ps0 = *ps1;
            wait();
        }
    }

    void sig_ptr2() {
        *qs0 = 0;        // q0 is reg
                         // q1 is comb (read-only)
        wait();
        
        while (true) {
            int j = qs0->read() + (*qs1).read();
            *qs0 = *qs1;
            wait();
        }
    }
    
    void port_ptr() {
        *qout = 0; 
        wait();
        
        while (true) {
            *qout = pin->read() + 1;
            wait();
        }
    }
    
// ----------------------------------------------------------------------------    
    
    void array_ptr1()
    {
        for (int i = 0; i < N; i++) {
            *parr1[i] = 0;
            *arr1[i] = i;
        }
        wait();
        
        while (1) {
            for (int i = 0; i < N; i++) {
                sc_biguint<100> mul = 0; 
                sc_bigint<100> dif = (sc_bigint<100>)*arr1[i];
                for (int j = 0; j < M; j++) {
                    mul *= *parr2[i][j] + iarr1[i]->read();
                    dif -= (*parr2[i][j]).read();
                }
                *parr1[i] = mul * dif;
            }
            
            wait();
        }
    }
    
    
    void array_ptr2()
    {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                *parr2[i][j] = 0;
                for (int k = 0; k < K; k++) {
                    *parr3[i][j][k] = 0;
                }
            }
        }
        *parr3[1][2][3] = 1;
        wait();
        
        while (1) {
            unsigned i = s.read();
            for (int j = 0; j < M; j++) {
                *parr2[i][j] = parr3[i][j][i+1]->read();
            }
            int j = s.read() ? i : 1;
            *parr3[3][i][1] = (int)s.read();
            *parr2[i][j] = (long int)s.read();
            
            wait();
        }
    }
    
    
};

int sc_main(int argc, char *argv[]) 
{
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<sc_uint<4>> sig;
    sc_signal<sc_uint<33>> sarr1[4];

    a_mod.clk(clk);
    a_mod.in(sig);
    a_mod.qout->bind(sig);
    for (int i = 0; i < 4; i++) {
        a_mod.iarr1[i]->bind(sarr1[i]);
    }
    
    sc_start();
    return 0;
}

