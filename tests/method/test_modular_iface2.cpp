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

// Modular interface with array of pointers/channels/variables inside
template<unsigned N>
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    
    sc_signal<bool>*    s[N];
    sc_signal<bool>     r[N];
    bool                v[N];
    bool                vv[N];

    SC_CTOR(mod_if) 
    {
        assert (N > 1);
        
        for (int i = 0; i < N; i++) {
            s[i] = new sc_signal<bool>("s[i]");
        }
        
        SC_METHOD(meth);
        sensitive << (*s[1]) << r[1];

        SC_CTHREAD(thread, clk.pos());
        async_reset_signal_is(rst, 1);
    }

    void meth() {
        v[1] = false;
        bool b = *s[1] || r[1] || v[1];
    }
    
    void thread() {
        bool c;
        vv[1] = false;
        wait();
        
        while(true) {
            c = *s[1] || r[1] || vv[1];
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
