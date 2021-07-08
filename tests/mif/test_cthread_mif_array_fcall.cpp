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

// Array of modular interface pointers with function call from parent THREAD
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk;
    sc_in<bool>     rst;
    
    sc_signal<sc_uint<4>>   s {"s"};
    sc_uint<4>              v;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(meth);
        sensitive << s;
    }
    
    void meth()
    {
        sc_uint<4> a = s;
    }

    void reset() {
        int l = 0;
        v = l;
        s = 0;
    }

    void write(sc_uint<4> val) {
        sc_uint<4> l = val;
        s = l + v;
    }

    sc_uint<4> read(sc_uint<4> val) {
        v = val;
        return s.read();
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  t;
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);
        }
        
        SC_CTHREAD(top_fcall1, clk.pos());
        async_reset_signal_is(rst, 1);
    }
    
    void top_fcall1() 
    {
        for (int i = 0; i < 2; i++) {
            minst[i]->reset();
        }
        wait();
        
        while (true) {
            int i = t;
            minst[i]->write(1);
            wait();
        }
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
