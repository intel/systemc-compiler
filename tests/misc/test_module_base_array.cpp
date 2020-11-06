/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Array of pointers to module base class
template <unsigned N>
struct A : public sc_module {
    sc_uint<N> a;

    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};

    SC_CTOR(A) 
    {}
};

template <unsigned N>
struct B : public A<N>
{
    SC_HAS_PROCESS(B);
    
    B (const sc_module_name& name) : A<N>(name) 
    {
        SC_CTHREAD(threadProc, this->clk.pos());
        this->async_reset_signal_is(this->rst, 0);
    }
    
    void threadProc() {
        this->a = 1;
        wait();
        
        while(true) {
            this->a++;
            wait();
        }
    }
};

struct Top : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    
    A<16>*  barr[3];
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < 3; ++i) {
            barr[i] = new B<16>("name");
            barr[i]->clk(clk);
            barr[i]->rst(rst);
        }
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    sc_signal<bool> rst{"rst"};
    Top top{"top"};
    top.clk(clock_gen);
    top.rst(rst);
    sc_start();

    return 0;
}

