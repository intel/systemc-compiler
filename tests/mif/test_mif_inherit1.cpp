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

// ...
template <unsigned N>
struct my_if : public sc_interface 
{
    virtual bool func() = 0;
};

template <unsigned N>
struct mod_if : public sc_module, public my_if<N> 
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

struct spec_mod_if : public mod_if<2>
{
    spec_mod_if(const sc_module_name& name) : mod_if<2>(name) 
    {}
};

SC_MODULE(top) {

    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    sc_signal<bool> s {"s"};

    spec_mod_if  minst1{"minst1"};
    spec_mod_if  minst2{"minst2"};

    SC_CTOR(top) 
    {
        minst1.rst(rst);
        minst1.clk(clk);
        minst2.rst(rst);
        minst2.clk(clk);
        
        SC_METHOD(top_method);
        sensitive << minst1.a << minst2.a;
    }

    void top_method() {
        s = minst1.a || minst2.a;
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
        top_inst.minst1.b(sig[1]);
        top_inst.minst2.a(sig[2]);
        top_inst.minst2.b(sig[3]);
    }
        

};

int sc_main(int argc, char **argv) {

    cout << "test_modular_iface_proc\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
