/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constants and static constants in MIF array,
// accessed from MIF and parent CTHREAD
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk;
    sc_signal<bool> rst;
    sc_in<int>      in;
    sc_signal<int>  t;
    
    static const unsigned A = 52;
    static constexpr unsigned B = 53;
    const unsigned C = 54;
    
    SC_CTOR(mod_if) 
    {
        SC_CTHREAD(mifProc, clk.pos());
        async_reset_signal_is(rst, 0);
    }
    
    void mifProc() 
    {
        const unsigned L = 56;
        unsigned i = A + B + C + L;
        wait();
        while (true) {
            unsigned j = A + B + C + L;
            t = i + j;
            wait();
        }
    }
};

SC_MODULE(Top) 
{
    sc_in_clk       clk{"clk"};
    mod_if*         aminst[2];
    
    sc_signal<int>  s;
    sc_signal<bool>  rst;

    SC_CTOR(Top) {
        aminst[0] = new mod_if("aminst0");
        aminst[1] = new mod_if("aminst1");
        aminst[0]->clk(clk);
        aminst[0]->in(s);
        aminst[1]->clk(clk);
        aminst[1]->in(s);
        
        SC_CTHREAD(topProc, clk.pos());
        async_reset_signal_is(rst, 0);
    }
    
    void topProc() 
    {
        int k = 0;
        for (int i = 0; i < 2; ++i) {
            k += aminst[i]->A + aminst[i]->B + aminst[i]->C;
        }
        int sum = k;
        wait();
        
        while (true) {
            int i = s.read();
            sum += aminst[i]->A + aminst[i]->B + aminst[i]->C;
            s = sum;
            wait();
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
