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

// Array of modular interface pointers accessed from parent module METHOD
struct mod_if : public sc_module, sc_interface 
{
    sc_signal<sc_uint<4>>   s {"s"};
    sc_uint<4>              v;
    sc_uint<4>              vv;
    sc_uint<4>              vvv;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(metProc);
        sensitive << s;
    }

    void metProc() {
        v = s.read();
    }
    
    void f() {
        vvv = s;
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
        }
        
        SC_METHOD(top_method);
        sensitive << minst[0]->s << minst[1]->s;

        SC_METHOD(top_method2);
        sensitive << t;
    }

    void top_method() 
    {
        minst[0]->f();
        sc_uint<4> a = minst[1]->s;
        minst[0]->s = a + minst[0]->vvv;
    }

    void top_method2() 
    {
        minst[1]->vv = t;
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
