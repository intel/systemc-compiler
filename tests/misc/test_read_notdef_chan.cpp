/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 09/20/19.
//

#include "sct_comb_signal.h"
#include <systemc.h>

// Used not defined channel variable, no error reported
template<unsigned N>
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    
    sc_signal<bool>         s; 
    sc_signal<bool>*        ss; 
    sct_comb_signal<bool>   cs; 
    sct_comb_signal<bool>*  css; 

    SC_CTOR(mod_if) 
    {
        ss = new sc_signal<bool>("ss");
        css = new sct_comb_signal<bool>("css");
        
        SC_METHOD(meth);
        sensitive << s << cs;

        SC_CTHREAD(thread, clk.pos());
        async_reset_signal_is(rst, 1);
    }

    void meth() {
        bool b = s || cs;
    }
    
    void thread() {
        bool c;
        wait();
        
        while(true) {
            c = ss->read();
            c = *css;
            wait();
            bool d = c;
        }
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    mod_if<2>       minst{"minst"};

    SC_CTOR(Top) {
        minst.clk(clk);
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
