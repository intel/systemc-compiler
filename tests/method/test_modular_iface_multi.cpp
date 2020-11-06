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

// Several instances of modular interface test
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    sc_signal<bool> s {"s"};
    sc_out<bool>    a {"a"};
    sc_out<bool>    b {"b"};
    sc_uint<2>      var;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(metProc);
        sensitive << s;

        SC_CTHREAD(thrProc, clk.pos());
        async_reset_signal_is(rst, 1);
    }

    void metProc() {
        var = !s.read();
        a = !var;
    }
    
    void thrProc() {
        sc_uint<3> i = 0;
        b = 0;
        wait();
        
        while (1) {
            b = i++;
            wait();
        }
    }

    bool func() {
        return s.read();
    }
};

SC_MODULE(top) {

    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    sc_signal<bool> s {"s"};

    mod_if  minst1{"minst1"};
    mod_if  minst2{"minst2"};

    SC_CTOR(top) 
    {
        minst1.rst(rst);
        minst1.clk(clk);
        minst2.rst(rst);
        minst2.clk(clk);
        
        SC_METHOD(top_method);
        sensitive << minst1.a << minst2.s;
    }

    void top_method() {
        s = minst1.a || minst2.func();
    }
};

SC_MODULE(tb) {

    sc_clock        clk {"clk", sc_time(1, SC_NS)};
    sc_signal<bool> rst {"rst"};

    sc_signal<bool> sig[4];

    top top_inst{"top_inst"};

    SC_CTOR(tb) {
        top_inst.clk(clk);
        top_inst.rst(rst);
    
        top_inst.minst1.a(sig[0]);
        top_inst.minst2.a(sig[1]);
        top_inst.minst1.b(sig[2]);
        top_inst.minst2.b(sig[3]);
    }
        

};

int sc_main(int argc, char **argv) {

    cout << "test_modular_iface_proc\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
