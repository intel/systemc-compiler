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

// Array of modular interface pointers with used and non-used signals
// Checking assignment statements generation
struct mod_if : public sc_module, sc_interface 
{
    sc_in<int>              in;
    sc_out<int>             out;
    sc_in<int>*             pin;
    sc_out<int>*            pout;

    SC_CTOR(mod_if) 
    {
        pin = new sc_in<int>("pin");
        pout = new sc_out<int>("pout");
        
        SC_METHOD(ptrProc);
        sensitive << in << *pin;
    }

    void ptrProc() {
        out = in;
        *pout = *pin;
    }
    
};

SC_MODULE(Top) 
{
    sc_in_clk       clk{"clk"};
    
    sc_signal<int>  t;   // Not used and therefore not assigned
    sc_signal<int>  z;   
    sc_signal<int>  pt;  
    sc_signal<int>  pz;  // Not used and therefore not assigned
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
        }
        minst[0]->in(t);
        minst[1]->in(z);
        minst[0]->out(t);
        minst[1]->out(z);

        minst[0]->pin->bind(pt);
        minst[1]->pin->bind(pz);
        minst[0]->pout->bind(pt);
        minst[1]->pout->bind(pz);
        
        SC_METHOD(top_meth);
        sensitive << z << pt;
    }
    
    void top_meth() {
        int i = z.read() + pt.read();
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
