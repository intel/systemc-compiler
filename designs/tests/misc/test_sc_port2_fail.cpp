/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Module with array of @sc_port to module object on stack -- not supported
template<typename T>
struct port_if : public sc_interface {
    virtual void f(T val) = 0;
};

template<typename T>
struct AhbSlave : public sc_module, sc_interface
{
    sc_in_clk       clk;
    sc_in<bool>     nrst;

    sc_port<port_if<T> >  slave_ports[2];
    
    SC_CTOR(AhbSlave) {
        SC_METHOD(methProc);
        sensitive << clk.pos();
    }
    
    void methProc() 
    {
        slave_ports[0]->f(0);
        slave_ports[1]->f(1);
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;

    using T = sc_uint<4>;
    sc_signal<T>        s;
    
    template<typename T>
    struct Target : public sc_module, port_if<T> 
    {
        Dut* parent;

        explicit Target (sc_module_name name, Dut* parent_) : 
            sc_module(name), 
            parent(parent_) 
        {}

        void f(T val) {
            parent->s = val;
        }
    };

    AhbSlave<T>         slave{"slave"};
    Target<T>           tar1{"tar1", this};
    Target<T>           tar2{"tar2", this};
    
    SC_HAS_PROCESS(Dut);
    
    explicit Dut (sc_module_name name) : sc_module(name) 
    {
        slave.clk(clk);
        slave.nrst(nrst);
        slave.slave_ports[0](tar1);
        slave.slave_ports[1](tar2);
    }
};


int sc_main(int argc, char **argv) 
{
    Dut dut{"dut"};
    sc_clock clk("clk", 1, SC_NS);
    dut.clk(clk);
    sc_start();
    return 0;
}
