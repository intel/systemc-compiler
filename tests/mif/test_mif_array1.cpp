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

// Array of modular interface instances 
struct mod_if : public sc_module//, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    sc_in<bool>     a {"a"};
    sc_out<bool>    b {"b"};
    sc_signal<bool> s {"s"};
    sc_uint<2>      var;

    SC_HAS_PROCESS(mod_if);
    
    mod_if(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(metProc);
        sensitive << a << s;

        SC_CTHREAD(thrProc, clk.pos());
        async_reset_signal_is(rst, 1);
    }

    void metProc() {
        var = a.read();
        b = s.read() && var;
    }

    void thrProc() {
        sc_uint<3> i = 0;
        wait();
        
        while (1) {
            s = i++;
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
    
    sc_signal<bool> a[2];

    mod_if          minst{"minst"}; 
    //mod_if          minst[2];

    SC_CTOR(top) 
    {
//        for (int i = 0; i < 2; i++) {
//            minst[i].rst(rst);
//            minst[i].clk(clk);
//            minst[i].a(a[i]);
//            minst[i].b(a[i]);
//        }
        
        minst.rst(rst);
        minst.clk(clk);
        minst.a(a[0]);
        minst.b(a[0]);
        
        //SC_METHOD(top_method);
        //sensitive << minst[0].s << minst[1].s;
    }

    void top_method() {
//        bool c = minst[0].s.read() || minst[1].func();
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
