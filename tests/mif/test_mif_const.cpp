/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constants and static constants in modular interface
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<int>      in;
    sc_signal<int>  t;
    
    static const unsigned A = 42;
    static const unsigned AA = 42;
    static const unsigned AAA = 42;
    const unsigned B = 43;
    static const unsigned C = 44;
    static const unsigned D = 45;
    
    static const unsigned N = C;   
    static const unsigned M = N+1;   

    SC_CTOR(mod_if) 
    {
        SC_METHOD(mifProc);
        sensitive << in;
    }
    
    void mifProc() 
    {
        unsigned i = N;
        if (M) {
            t = i;
        }
    }
};

SC_MODULE(Top) 
{
    sc_in_clk       clk{"clk"};
    mod_if          minst{"minst"};
    
    sc_signal<int>  s;
    sc_signal<int>  t;

    SC_CTOR(Top) {
        minst.clk(clk);
        minst.in(s);
        
        SC_METHOD(topProc);
        sensitive << s;
    }
    
    void topProc() 
    {
        unsigned i;
        i = (minst.B) ? mod_if::A : 1;
        i = mod_if::AA;          // OK
        i = minst.AAA;          // Bug, no minst_AA declared, #249
        
        if (mod_if::C) {
            t = i + mod_if::D;
        }
    }
    
};

int sc_main(int argc, char **argv) 
{
    sc_clock  clk("clk", sc_time(1, SC_NS));
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
