/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Record parameters of function passed by value and by reference
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) 
    {
        SC_METHOD(record_fcall_ref);  
        sensitive << dummy;
                
        SC_METHOD(record_fcall_val);  
        sensitive << dummy;
                
        SC_METHOD(record_fcall_two_val);  
        sensitive << dummy;

        SC_METHOD(record_fcall_two_val2);  
        sensitive << dummy;

        SC_METHOD(record_fcall_two_ref);  
        sensitive << dummy;

        SC_METHOD(record_fcall_two_ref2);  
        sensitive << dummy;

        SC_METHOD(record_fcall_mixed);  
        sensitive << dummy;

        SC_METHOD(record_two_fcalls_mixed);  
        sensitive << dummy;
                
        
        SC_METHOD(record_fcall_const_ref1);  
        sensitive << dummy;

        SC_METHOD(record_fcall_const_ref2);  
        sensitive << dummy;

        SC_METHOD(record_fcall_const_ref3);  
        sensitive << dummy;
        
    }
    
    struct Simple {
        bool a;
        int b;
    };
    
    void f1(Simple& par) {
        bool b = par.a;
        par.b = 2;
    }
    
    void f1_const(const Simple& par) {
        bool b = !par.a;
    }
    
    void f2(Simple par) {
        bool b = par.a;
        par.b = 2;
    }
    
    void f3(Simple par1, Simple par2) {
        bool b = par1.a || par2.a;
        par1.a = b+1;
        par2.a = b-1;
    }
    
    void f4(Simple& par1, const Simple& par2) {
        bool b = par1.a && par2.a;
        par1.a = b;
    }

    // Parameter by reference and constant reference
    void record_fcall_ref() 
    {
        Simple s;
        s.b = 1;
        f1(s);       
        f1_const(s); 
        
        sct_assert_read(s.a);
        sct_assert_defined(s.b);
    }

    // Parameter by value
    void record_fcall_val() 
    {
        Simple s;
        s.b = 1;
        f2(s);
    }

    // Two record parameters by value
    void record_fcall_two_val() 
    {
        Simple s; Simple r;
        f3(s, r);
    }
    
    // Global and local record parameters by value
    Simple gr;
    void record_fcall_two_val2() 
    {
        Simple s;
        gr.a = true;
        f3(gr, s);
    }
    
    // Two record parameters by reference
    void record_fcall_two_ref() 
    {
        Simple s; Simple r;
        f4(s, r);
    }
    
    // Global and local record parameters by reference
    Simple gs;
    void record_fcall_two_ref2() 
    {
        Simple r;
        r.b = 4;
        gs.a = true;
        f4(gs, r);
    }

//--------------------------------------------------------------------------
    int f5(int par1, Simple par2, bool par3) {
        int i = par3 ? par1 : par2.b;
        return i;
    }
    
    void f5a(int par1, bool par3) {
    }
    
    // Record and non-record parameters by reference
    void record_fcall_mixed() 
    {
        Simple r;
        int j; bool b;
        f5(j, r, b);
        f5(42, r, true);
    }
    
    // Two function calls with different parameters
    void record_two_fcalls_mixed()
    {
        Simple s;
        Simple r;
        f3(s, r);
        f3(r, s);
        int j = 42; bool b;
        r.a = true;
        f5(j, r, b);
        r.b = 11;
        int k = f5(j, s, b);
    }
    
// ---------------------------------------------------------------------------    
    // Constant reference parameters
    
    void cref_copy(Simple& par1, const Simple& par2) {
        par1.a = par2.a; par1.b = par2.b;
    }
    
    void record_fcall_const_ref1() 
    {
        Simple r; Simple t;
        cref_copy(r, t);
    }
    

    bool cref_cmp(const Simple& par1, const Simple& par2) {
        return (par1.a == par2.a && par1.b == par2.b);
    }

    void record_fcall_const_ref2() 
    {
        Simple r; Simple t;
        t.a = 1;
        bool b = cref_cmp(r,t);
    }
    
    int cref_sum(const Simple& par) {
        int res = par.a + par.b;
        return res;
    }

    void record_fcall_const_ref3() 
    {
        Simple r; 
        Simple t;
        int i = cref_sum(t);
        i = cref_sum(r);
    }
};

class B_top : public sc_module {
public:
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

