/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Check unused variables/statements remove for signals and ports
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;
    sc_signal<sc_uint<4>> r;

    // Ports never removed
    sc_in<int>   i1;
    sc_out<int>  o1;
    sc_out<int>* o2;
    sc_in<int>   iarr1[3];
    sc_out<int>  oarr1[3];
    sc_in<int>   iarr2[3][2];
    sc_out<int>* oarr2[3][2];
    sc_vector<sc_in<int>> ivec1{"ivec1", 3};
    sc_vector<sc_vector<sc_out<int>>> ovec2{"ovec2", 3};
    
    sc_signal<int>  t0;
    sc_signal<int>  t1;
    sc_signal<int>  t2;
    sc_signal<int>  t3;             // removed
    sc_signal<int>* tp1;
    sc_signal<int>* tp2;
    sc_signal<int>* tp3;            // removed
    
    sc_signal<int> tarr0[3];
    sc_signal<int> tarr1[3];        
    sc_signal<int> tarr2[3];        // removed    
    sc_signal<int> tarr3[3][2];
    sc_signal<int> tarr4[3][2];
    sc_signal<int> tarr5[3][2];     // removed
    sc_signal<int>* parr1[3];
    sc_signal<int>* parr2[3][2];
    
    sc_vector<sc_signal<int>> vec0{"vec0", 3};
    sc_vector<sc_signal<sc_uint<4>>> vec1{"vec1", 3};
    sc_vector<sc_signal<sc_uint<4>>> vec2{"vec2", 3};   // removed    
    sc_vector<sc_signal<sc_uint<4>>> vec3{"vec2", 3};
    sc_vector<sc_vector<sc_signal<int>>> vvec0{"vvec0", 3};
    sc_vector<sc_vector<sc_signal<int>>> vvec1{"vvec1", 3};
    sc_vector<sc_vector<sc_signal<sc_uint<4>>>> vvec2{"vvec2", 3};  // removed    
    sc_vector<sc_vector<sc_signal<sc_uint<4>>>> vvec3{"vvec3", 3};  // removed    

    sc_vector<sc_signal<int>> vec0t{"vec0", 3};
    sc_vector<sc_signal<sc_uint<4>>> vec1t{"vec1", 3};
    sc_vector<sc_signal<sc_uint<4>>> vec2t{"vec2", 3};   // removed    
    sc_vector<sc_signal<sc_uint<4>>> vec3t{"vec2", 3};
    sc_vector<sc_vector<sc_signal<int>>> vvec0t{"vvec0", 3};
    sc_vector<sc_vector<sc_signal<int>>> vvec1t{"vvec1", 3};
    sc_vector<sc_vector<sc_signal<sc_uint<4>>>> vvec2t{"vvec2", 3};  // removed    
    sc_vector<sc_vector<sc_signal<sc_uint<4>>>> vvec3t{"vvec3", 3};  // removed    

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        
        tp1 = new sc_signal<int>("tp1");
        tp2 = new sc_signal<int>("tp2");
        tp3 = new sc_signal<int>("tp3");
        o2 = new sc_out<int>("o2");
        for (int i = 0; i < 3; i++) {
            ovec2[i].init(2);
            vvec0[i].init(2);
            vvec1[i].init(2);
            vvec2[i].init(2);
            vvec3[i].init(2);
            vvec0t[i].init(2);
            vvec1t[i].init(2);
            vvec2t[i].init(2);
            vvec3t[i].init(2);
            parr1[i] = new sc_signal<int>("parr1");
            for (int j = 0; j < 2; j++) {
                parr2[i][j] = new sc_signal<int>("parr2");
                oarr2[i][j] = new sc_out<int>("oarr2");
            }
        }
        
        SC_METHOD(remove_port); 
        sensitive << s << r << i1 << o1 << *o2;
        for (int j = 0; j < 3; j++) {
            sensitive << iarr1[j] << oarr1[j] << ivec1[j];
        }

        SC_METHOD(remove_sig); 
        sensitive << s << r << t0 << *tp1;
        
        SC_METHOD(remove_sig_arr); sensitive << s << r;
        for (int j = 0; j < 3; j++) {
            sensitive << tarr0[j] << *parr1[j];
            for (int k = 0; k < 2; k++) {
                sensitive << tarr3[j][k] << *parr2[j][k];
            }
        }
        
        SC_METHOD(remove_sig_vec); sensitive << s << r;
        for (int j = 0; j < 3; j++) {
            sensitive << vec0[j];
            for (int k = 0; k < 2; k++) {
                sensitive << vvec0[j][k] << vvec3[j][k];
            }
        }
        
        SC_CTHREAD(remove_sig_arr_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_sig_vec_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    
    // Ports, port array and vectors
    void remove_port() {
        // Do nothing, ports not removed
    }
    
    // Signal and pointer to signal
    void remove_sig() 
    {
        int i = 1;
        int j = t0;
        t1 = i + j;
        t2 = *tp1;
        *tp2 = 42;
    }
    
    void remove_sig_arr() {
        int i;
        i = tarr0[r.read()];
        tarr1[s.read()] = i;
        tarr4[i][i] = 2;
        i = parr1[i]->read();
        parr2[i][s.read()]->write(tarr3[s.read()][1]);
        
    }
    
    sc_signal<int> tarr0t[3];
    sc_signal<int> tarr1t[3];        
    sc_signal<int> tarr2t[3];        // removed    
    sc_signal<int> tarr3t[3][2];
    sc_signal<int> tarr4t[3][2];     // removed
    
    void remove_sig_arr_thread() 
    {
        wait();

        while (true) {
            int i;
            i = tarr0t[r.read()];
            tarr1t[s.read()] = i;
            tarr4t[i][i] = 2;
            wait();
        }
    }
    
    // Signal 1D and 2D vectors
    void remove_sig_vec() 
    {
        const int& l = vec0[1].read();
        vec1[l] = 2;
        vec3[r.read()] = vvec0[1][1].read();
        vvec1[1][l].write(42);
        sc_uint<4> ll = vvec3[s.read()][l];     // removed
    }

    void remove_sig_vec_thread() 
    {
        wait();

        while (true) {
            const int& l = vec0t[1].read();
            vec1t[l] = 2;
            vec3t[r.read()] = vvec0t[1][1].read();
            vvec1t[1][l].write(42);
            sc_uint<4> ll = vvec3t[s.read()][l];     // removed
            wait();
        }
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_signal<int>   i1;
    sc_signal<int>   o1;
    sc_signal<int>   o2;
    sc_signal<int>   iarr1[3];
    sc_signal<int>   oarr1[3];
    sc_signal<int>   iarr2[3][2];
    sc_signal<int>   oarr2[3][2];
    sc_signal<int>   ivec1[3];
    sc_signal<int>   ovec2[3][2];
    
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    a_mod.i1(i1);
    a_mod.o1(o1);
    a_mod.o2->bind(o2);
    for (int i = 0; i < 3; i++) {
        a_mod.iarr1[i](iarr1[i]);
        a_mod.oarr1[i](oarr1[i]);
        a_mod.ivec1[i](ivec1[i]);
        for (int j = 0; j < 2; j++) {
            a_mod.oarr2[i][j]->bind(oarr2[i][j]);
            a_mod.ovec2[i][j](ovec2[i][j]);
        }
    }
    for (int i = 0; i < 3; i++) {
        a_mod.iarr2[i][0](iarr2[i][0]);
        a_mod.iarr2[i][1](iarr2[i][1]);
    }
    
    sc_start();
    return 0;
}

