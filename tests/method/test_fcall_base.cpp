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

using namespace sc_core;

#define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

// Function calls of base class function
struct A : sc_module 
{
    SC_HAS_PROCESS(A);
    
    sc_signal<int> s;
    
    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(virt_meth0);
        sensitive << s;

        SC_METHOD(meth0);
        sensitive << s;
    }
 
    virtual int vf(int i)  {
        return (i+1);
    }
    
    int f(int i)  {
        return (i+1);
    }

    virtual int vff(int i)  {
        return (i+1);
    }
    
    int ff(int i)  {
        return (i+1);
    }
    
    void virt_meth0() 
    {
        int k;
        k = A::vf(1);
        CHECK(k == 2);
        k = vf(1);
        CHECK(k == 3);
        k = this->vf(1);
        CHECK(k == 3);
    }
    
    void meth0() 
    {
        int k;
        k = A::f(1);
        CHECK(k == 2);
        k = f(1);
        CHECK(k == 2);
        k = this->f(1);
        CHECK(k == 2);
    }
};

struct B : A
{
    SC_HAS_PROCESS(B);
    
    sc_signal<int> s;
    
    B(const sc_module_name& name) : A(name) 
    {
        SC_METHOD(virt_meth1);
        sensitive << s;

        SC_METHOD(meth1);
        sensitive << s;

        SC_METHOD(meth2);
        sensitive << s;
    }
 
    int vf(int i)  {
        return (i+2);
    }
    
    int f(int i)  {
        return (i+2);
    }
    
    int vff(int i)  {
        int j = A::vff(i);
        return (j+2);
    }
    
    int ff(int i)  {
        int j = A::ff(i);
        return (j+2);
    }
    
    void virt_meth1() 
    {
        int k;
        k = A::vf(1);
        CHECK(k == 2);

        k = B::vf(1);
        CHECK(k == 3);
        k = this->vf(1);
        CHECK(k == 3);
        k = vf(1);
        CHECK(k == 3);
        k = ((A*)(this))->vf(1);
        CHECK(k == 3);
    }

    void meth1() 
    {
        int k;
        k = A::f(1);
        CHECK(k == 2);

        k = B::f(1);
        CHECK(k == 3);
        k = this->f(1);
        CHECK(k == 3);
        k = f(1);
        CHECK(k == 3);
        k = ((A*)(this))->f(1);
        CHECK(k == 2);
    }
    
    void meth2()
    {
        int k;
        k = vff(1);
        CHECK(k == 4);
        k = ff(1);
        CHECK(k == 4);
    }
    
};

int sc_main(int argc, char *argv[]) 
{
    B b_mod{"b_mod"};
    sc_start();
    return 0;
}

