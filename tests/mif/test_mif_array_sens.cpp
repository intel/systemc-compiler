/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 06/14/19.
//

#include <systemc.h>

// Array of modular interface pointers used in METHOD sensitivity list
struct mod_if : public sc_module, sc_interface 
{
    sc_signal<bool>   b;
    
    sc_signal<unsigned>   s1;
    sc_signal<unsigned>   s2;
    sc_signal<unsigned>   s3;
    sc_signal<unsigned>   r;
    
    SC_HAS_PROCESS(mod_if);
    
    mod_if(const sc_module_name& name, bool par_) : 
        sc_module(name), B(par_) 
    {
        par = par_;
    }
    
    void before_end_of_elaboration() override {
        C = sc_new<bool>(par);
    }
     
    bool par;
    const bool A = true;
    const bool B;
    const bool* C = nullptr;
    
    void f1() {
        if (A) {
            s1 = 1;
        } else {
            s1 = r.read();
        }
    }
    
    void f2() {
        if (B) {
            s2 = 1;
        } else {
            s2 = r.read();
        }
    }
    
    void f3() {
        if (C) {
            s3 = 1;
        } else {
            s3 = r.read();
        }
    }
};

SC_MODULE(Top) {

    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    sc_signal<unsigned> t;
    mod_if*             minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if", true);
        }
        
        SC_METHOD (top_method1);
        sensitive << t;

        SC_METHOD (top_method2);
        sensitive << t;

        // TODO: fix me #279
        SC_METHOD (top_method2_bug);       
        sensitive << t;

        SC_METHOD (top_method3);
        sensitive << t;
    }
    
    void top_method1() {
        unsigned i = t.read();
        minst[i]->f1();
    }

    void top_method2() {
        unsigned i = t.read();
        //minst[i]->f2();
        if (minst[i]->B) {
            minst[i]->s2 = 1;
        } else {
            minst[i]->s2 = minst[i]->r.read();
        }
    }
    
    // TODO: fix me #279
    void top_method2_bug() {
        unsigned i = t.read();
        minst[i]->f2();             // if (minst_0_B[i]) generated !!!
    }

    void top_method3() {
        unsigned i = t.read();
        minst[i]->f3();
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
