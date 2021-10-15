/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Two instances of different MIF, one accessed from another one
struct mod_if1 : public sc_module, sc_interface 
{
    sc_signal<bool> s{"s"};
    sc_out<bool>    a{"a"};
    sc_uint<2>      var;

    SC_CTOR(mod_if1) 
    {
        SC_METHOD(mifMethod);
        sensitive << s;
    }

    void mifMethod() {
        a = !s.read();
    }

    bool func() {
        return (s.read()+1);
    }
};

struct mod_if2 : public sc_module, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    sc_out<bool>    b{"b"};
    mod_if1*        mp;
    
    SC_HAS_PROCESS(mod_if2);

    mod_if2(const sc_module_name& name, mod_if1* mp_) : sc_module(name), mp(mp_)
    {
        SC_CTHREAD(mifThread, clk.pos());
        async_reset_signal_is(rst, 1);
    }
    
    void mifThread() 
    {
        mp->var = 0;
        mp->s = 1;
        b = 0;
        wait();
        
        while (1) {
            if (mp->s.read() == mp->var) b = mp->func();
            wait();
        }
    }
};

SC_MODULE(top) {

    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    sc_signal<bool> a {"a"};
    sc_signal<bool> s {"s"};

    mod_if1  minst1{"minst1"};
    mod_if2  minst2{"minst2", &minst1};

    SC_CTOR(top) 
    {
        minst1.a(a);
        minst2.rst(rst);
        minst2.clk(clk);
        
        SC_METHOD(topMethod);
        sensitive << a << minst1.s << minst2.b;
    }

    void topMethod() {
        s = a || minst1.func() || minst2.b;
    }
};

SC_MODULE(tb) {

    sc_clock        clk {"clk", sc_time(1, SC_NS)};
    sc_signal<bool> rst {"rst"};

    sc_signal<bool> s;

    top top_inst{"top_inst"};

    SC_CTOR(tb) {
        top_inst.clk(clk);
        top_inst.rst(rst);
    
        top_inst.minst2.b(s);
    }
};

int sc_main(int argc, char **argv) {

    cout << "test_modular_iface_proc\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
