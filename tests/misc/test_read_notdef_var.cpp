/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 09/20/19.
//

#include <systemc.h>

// Used not defined non-channel variable, 2x errors reported
template<unsigned N>
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    
    bool                v; 
    bool                vv; 

    SC_CTOR(mod_if) 
    {
        SC_METHOD(meth);
        sensitive << rst;

        SC_CTHREAD(thread, clk.pos());
        async_reset_signal_is(rst, 1);
        
        v = 1;
        vv = 1;
    }

    void meth() {
        bool b = v;
    }
    
    void thread() {
        bool c;
        wait();
        
        while(true) {
            c = vv;
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
