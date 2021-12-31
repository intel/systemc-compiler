/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/


#include <systemc.h>

// Check constant/pointer nullptr value in MIF array element accessed 
// by unknown index, check in IF condition
struct A_if : public sc_module, sc_interface 
{
    A_if(const sc_module_name& name) : 
        sc_module(name)
    {}
    
    sc_signal<unsigned>   s;
    void f() {
        s = 1;
    }
};

struct B_if : public sc_module, sc_interface 
{
    sc_signal<unsigned>   r;
 
    SC_HAS_PROCESS(B_if);
    
    B_if(const sc_module_name& name, bool par) : 
        sc_module(name), N(par ? 2 : 3)
    {
        if (par) 
            a = new A_if("a");
    }
    
    A_if* a = nullptr;
    
    void g() {
        int k;
        if (a) {
            a->f();
        } else {
            k = 1;
        }
    }
    
    const unsigned N;
    
    void f() {
        int k = 0;
        for (int i = 0; i < N; ++i) {
            k += i;
        }
    }
};

SC_MODULE(Top) {

    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    sc_signal<unsigned>   t;
    
    B_if* b[2];
    B_if* bb[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            b[i] = new B_if("b", 0);
            bb[i] = new B_if("bb", 1);
        }
        
        SC_METHOD (topMeth); sensitive << t;
        SC_METHOD (topMeth1); sensitive << t;
        SC_METHOD (topMeth2); sensitive << t;
    }
    
    // Check pointer in IF condition
    void topMeth() {
        unsigned i = t.read();
        for (int i = 0; i < 2; ++i) b[i]->g();
        b[0]->g();
        b[1]->g();
        b[i]->g();
        
        for (int i = 0; i < 2; ++i) bb[i]->g();
        bb[0]->g();
        bb[1]->g();
        bb[i]->g();
    }
    
    // Normal array not affected
    void topMeth1() {
        unsigned i = t.read();
        int m[3] = {1,2,3};
        if (m[i])
            i = 1;
    }
    
    // Check constant in loop
    void topMeth2() {
        unsigned i = t.read();
        b[i]->f();
        bb[i]->f();
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
