/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constant pointers, record and constant record in module and MIF
struct Rec {
    sc_uint<4> a;
    bool b;
    int c;
};

struct mod_if : public sc_module, sc_interface 
{
    sc_signal<unsigned>   s;
    Rec r;
    const Rec CR = {9, true, -2};
    
    SC_HAS_PROCESS(mod_if);
    
    mod_if(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD (var_rec); sensitive << s;
    }
    
    void var_rec() {
        r.a = s.read()+1;
        r.b = r.a * CR.a;
        int l = s.read() ? r.c : r.a.to_int();
    }
};

SC_MODULE(Top) {

    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    sc_signal<unsigned>   t;
    
    const Rec R = {9, true, -2};
    const unsigned  ca[2] = {11,12};
    //const unsigned* cpa[2] = {};          -- not supported yet
    
    int i = 42;
    const int* pi = &i;
    const int c = 43;
    const int* pc = &c;
    const int* pd;
    
    mod_if* mif[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            mif[i] = new mod_if("mif");
        }
        pd = sc_new<int>(44);
        
//        for (int i = 0; i < 2; i++) {
//            cpa[i] = sc_new<unsigned>(2*i+1);
//        }
        
        SC_METHOD (const_ptr); sensitive << t;
        SC_METHOD (const_ptr_arr); sensitive << t;
        SC_METHOD (const_rec); sensitive << t;
        SC_METHOD (mif_rec); sensitive << t;
    }
    
    void const_ptr() {
        int l;
        i = 41;
        l = *pi;
        l = *pc;
        l = *pd;
    }

    void const_ptr_arr() {
        unsigned j = t.read();
        unsigned lu;
        lu = ca[j];
    }
    
    void const_rec() {
        int k = R.b ? R.c : t.read();
        k = R.a.to_int();
    }

    void mif_rec() {
        int k;
        unsigned ii = t.read();

        k = mif[0]->r.a;
        k = mif[ii]->r.c;
        
        k = mif[0]->CR.c;
        k = mif[ii]->CR.a.to_int();
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
