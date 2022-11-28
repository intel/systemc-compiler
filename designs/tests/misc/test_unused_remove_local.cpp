/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Check unused variables/statements leads to local variables removed in SV
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;

    sc_in<int>   i1;
    sc_out<int>  o1;
    sc_out<int>* o2;
    sc_in<int>   iarr1[3];
    sc_out<int>  oarr1[3];
    sc_in<int>   iarr2[2][1];
    sc_out<int>* oarr2[3][4];
    sc_vector<sc_in<int>> ivec1{"ivec1", 3};
    sc_vector<sc_vector<sc_out<int>>> ovec2{"ovec2", 3};
    
    sc_signal<int>  t1;
    sc_signal<int>  t2;
    sc_signal<int>* tp1;
    sc_signal<int>* tp2;
    sc_signal<int> tarr1[3];
    sc_signal<int> tarr2[3][3];
    sc_signal<int>* parr1[3];
    sc_signal<int>* parr2[3][4];
    sc_vector<sc_signal<int>> vec{"vec", 3};
    sc_vector<sc_vector<sc_signal<int>>> vec2{"vec2", 3};
    sc_vector<sc_signal<sc_uint<4>>> vec3{"vec3", 3};

    int m; int ma;
    int* pm = &m;
    int* pma = &ma;
    int arr[3][3];
    int arra[3][3];
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        
        tp1 = new sc_signal<int>("tp1");
        tp2 = new sc_signal<int>("tp2");
        o2 = new sc_out<int>("o2");
        for (int i = 0; i < 3; i++) {
            vec2[i].init(4);
            ovec2[i].init(4);
            parr1[i] = new sc_signal<int>("parr1");
            for (int j = 0; j < 4; j++) {
                parr2[i][j] = new sc_signal<int>("parr2");
                oarr2[i][j] = new sc_out<int>("oarr2");
            }
        }
        
        SC_METHOD(remove0); sensitive << s;
        SC_METHOD(remove1); sensitive << s;
        SC_METHOD(remove1a); sensitive << s;
        SC_METHOD(remove2); sensitive << s << i1;
        SC_METHOD(remove2a); sensitive << s;
        SC_METHOD(remove3); sensitive << s;
        SC_METHOD(remove4); sensitive << s << i1 << ivec1[0];
        SC_METHOD(remove5); sensitive << s;
        SC_METHOD(remove6); 
        sensitive << s << i1 << iarr1[1] << iarr2[0][0] << iarr2[1][0];
        SC_METHOD(remove7); sensitive << s << i1;
        SC_METHOD(remove8); sensitive << s;
        SC_CTHREAD(remove9, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    
    int m1;
    void remove_member1() {
        int k = 1;
        m1 = 1;
    }
    
// ---------------------------------------------------------------------------    
    
    void f(int& l) {
        l++;
    }
    
    int g(int l) {
        return l+1;
    }
    
    // All the variables removed
    void remove0() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = 2; 
        
        int xn; 
        sc_uint<4> yn;

        int i = 1;
        int j = i + 1;
    }
    
    // Side effect operator
    void remove1() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = x; 
        
        --x;
        
        int i = 1;
        int j = i + 1;
        
        int l = j++;
    }
    
    // All the variables removed
    void remove1a() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = x; 
        
        int i = 1;
        int j = i + 1;
        
        int l = -j;
    }
    
    // Signal write and assign
    void remove2() 
    {
        int i = 1;
        int j = i + 1;

        *pm = j;
        arr[1][j] = m;
        t1.write(arr[s.read()][s.read()]);

        int l = s.read();
        t2 = l * l;
        
        int e1 = i1.read();
        int e2 = 1;
        o1 = e2;
        
        auto e3 = i1.read()+1;
        *o2 = e3;
    }    
    
    // No write to signal -- all variables removed
    void remove2a() 
    {
        int i = 1;
        int j = i + 1;

        *pma = j;
        arra[1][j] = ma;
    }    

    // Function call
    void remove3() 
    {
        int i = 1;
        int j = i + 1;
        
        f(j);
        int k = g(j);

    }    

    // Signal 1D and 2D vectors
    void remove4() 
    {
        int i = 1;
        int j = i + 1;
        
        vec[1] = (j || s.read()) ? 1 : 2;
        
        sc_uint<4> x = 2;
        vec2[0][s.read()] = -x;
        
        sc_biguint<44> y = (sc_biguint<44>)s.read();
        vec2[s.read()][s.read()+1] = y.to_int() - 42;
        
        sc_int<14> z = (sc_int<14>)s.read();
        vec3[s.read()+1] = (sc_uint<4>)z;
        
        int e1 = i1.read();
        int e2 = ivec1[e1];
        ovec2[s.read()][1] = e2;
    }

    
    // Pointer to signal
    void remove5() 
    {
        int i = 1;
        int j = i + 1;
        
        *tp1 = 1+ (1+j);
        
        int l = s.read();
        tp2->write(l);
    }
    
    // Signal array
    void remove6() 
    {
        int i = 1;
        int j = i + 1;
        
        tarr1[1] = j;
        
        int l = s.read();
        tarr1[s.read()] = l;
        
        sc_uint<4> x = 2*s.read();
        tarr2[2][s.read()] = x;

        sc_int<16> y = s.read() + 1;
        tarr2[s.read()][1] = y;
        
        int e1 = i1.read();
        int e2 = iarr1[1] + iarr2[s.read()][e1];
        oarr1[s.read()+1] = e2;
    }
    
    // Signal pointer array
    void remove7() 
    {
        int i = 1;
        int j = i + 1;
        
        *parr1[1+s.read()] = j;
        
        int l = s.read();
        *parr1[2] = l;

        sc_uint<4> x = 2*s.read();
        *parr2[2][s.read()] = x;

        sc_int<16> y = s.read() + 1;
        *parr2[s.read()][2*s.read()] = y;
       
        int e1 = i1.read();
        int e2 = i1.read()+1;
        *oarr2[s.read()][s.read()] = e2;
    }

    // Loops, side effect in loop increment
    void remove8() 
    {
        int i = 1;
        int j = i + 1;
        
        for (int k = 0; k < 10; k+=j) {
            i++;
        }
    }
    
    // Assertions
    void remove9() 
    {
        int i = 1;
        int j = s.read();
        int k = s.read()+1;
        
        SCT_ASSERT_THREAD(i && !j, SCT_TIME(1), s.read(), clk.pos());
        wait();
        
        SCT_ASSERT_THREAD(k == 42, (1,2), t1, clk.pos());
        
        while (1) {
            
            int l = s.read();
            
            wait();

            sct_assert(l == s.read());
        }
    }
    
    const int f1 = 42;
    const int f2 = 43;
    SCT_ASSERT(t1 == f1, (0), t2, clk.pos());
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_signal<int>   i1;
    sc_signal<int>   o1;
    sc_signal<int>   o2;
    sc_signal<int>   iarr1[3];
    sc_signal<int>   oarr1[3];
    sc_signal<int>   iarr2[2][1];
    sc_signal<int>   oarr2[3][4];
    sc_signal<int>   ivec1[3];
    sc_signal<int>   ovec2[3][4];
    
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
        for (int j = 0; j < 4; j++) {
            a_mod.oarr2[i][j]->bind(oarr2[i][j]);
            a_mod.ovec2[i][j](ovec2[i][j]);
        }
    }
    for (int i = 0; i < 2; i++) {
        a_mod.iarr2[i][0](iarr2[i][0]);
    }
    
    sc_start();
    return 0;
}

