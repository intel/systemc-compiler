/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Pure virtual function call in the module class with overloaded functions

template<class T>
struct B : public sc_module
{
    sc_in_clk   clk{"clk"};
    sc_in<bool> rst{"rst"};
    
    B(const sc_module_name& name) : sc_module(name) {
    }
};

template<class T>
struct A : public B<T>
{
    T  m;
    sc_signal<bool> dummy{"dummy"};

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : B<T>(name) {
        
        //SC_METHOD(virt_call); sensitive << dummy;
        
        SC_CTHREAD(virt_call, this->clk.pos()); 
        this->async_reset_signal_is(this->rst, false);
    }
    
    virtual void d() = 0;
    virtual void f(int i) = 0;
    
    void virt_call() {
        d();
        f(12);
        wait();
        
        while(1) wait();
    }
};


template<class T, unsigned>
class C 
{};

template<class T>
class C<T, 1> : public A<T>
{
public:
    
    C(const sc_module_name& name) : A<T>(name) 
    {}
    
    virtual void d() {
        this->m = 1;
    }
    virtual void f(int i) {
        this->m = i;
    }
};


class B_top : public sc_module 
{
public:
    sc_signal<bool>   clk{"clk"};
    sc_signal<bool>   rst{"rst"};

    C<int, 1> c_mod{"c_mod"};
    
    B_top(const sc_module_name& name) : sc_module(name) {
        c_mod.clk(clk);
        c_mod.rst(rst);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
