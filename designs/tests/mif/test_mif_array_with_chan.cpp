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

// Array of modular interface pointers with signal declaration 
struct mod_if : public sc_module, sc_interface 
{
    sc_signal<bool>     s{"s"};

    SC_CTOR(mod_if) 
    {
        SC_METHOD(metProc);
        sensitive << s;
    }

    void metProc() {
        bool d = s;
    }
    
};

SC_MODULE(top) {

    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    mod_if*         minst[2];

    SC_CTOR(top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            //minst[i]->clk(clk);
            //minst[i]->rst(rst);
        }
    }

};

SC_MODULE(tb) {

    sc_clock        clk {"clk", sc_time(1, SC_NS)};
    sc_signal<bool> rst {"rst"};

    top top_inst{"top_inst"};

    SC_CTOR(tb) {
        top_inst.clk(clk);
        top_inst.rst(rst);
    }

};

int sc_main(int argc, char **argv) {

    cout << "test_modular_iface_proc\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
