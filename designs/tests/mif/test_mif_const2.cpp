/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constants and static constants in MIF instance and MIF array,
// accessed from MIF and parent METHOD
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
        unsigned i = N + A + B;
        if (M) {
            t = i;
        }
    }
};

struct mod_if2 : public sc_module, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<int>      in;
    sc_signal<int>  t;
    
    static const unsigned A = 52;
    static constexpr unsigned B = 53;
    const unsigned C = 54;

    SC_CTOR(mod_if2) 
    {
        SC_METHOD(mifProc);
        sensitive << in;
    }
    
    void mifProc() 
    {
        const unsigned L = 56;
        unsigned i = B + C + L;
        t = i;
    }
};

SC_MODULE(Top) 
{
    sc_in_clk       clk{"clk"};
    mod_if          minst{"minst"};
    mod_if*         pminst;
    mod_if2*        aminst[2];
    
    sc_signal<int>  s;
    sc_signal<int>  t;

    SC_CTOR(Top) {
        pminst = new mod_if("pminst");
        
        minst.clk(clk);
        minst.in(s);
        pminst->clk(clk);
        pminst->in(s);

        aminst[0] = new mod_if2("aminst0");
        aminst[1] = new mod_if2("aminst1");
        aminst[0]->clk(clk);
        aminst[0]->in(s);
        aminst[1]->clk(clk);
        aminst[1]->in(s);
        
        SC_METHOD(topProc);
        sensitive << s;

        SC_METHOD(topProcPtr);
        sensitive << s;
        
        SC_METHOD(topProcArr);
        sensitive << s;
    }
    
    void topProc() 
    {
        unsigned i;
        i = (minst.B == 43) ? mod_if::A : 1;
        i = mod_if::AA;          
        i = minst.AAA;          
        
        if (mod_if::C) {
            t = i + mod_if::D;
        }
    }
    
    void topProcPtr() 
    {
        unsigned i;
        if (pminst->B == 43) {
            int i = (*pminst).A + pminst->AA;
        }
    }
    
    void topProcArr() 
    {
        unsigned i;
        if (aminst[0]->B) {
            int i = aminst[0]->A + aminst[1]->B + aminst[1]->C;
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
