/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Module with vector of @sc_port -- not supported
template<typename T>
struct port_if : public sc_interface {
    virtual void f(T val) = 0;
};

template<typename T>
struct Target : public sc_module, port_if<T> 
{
    sc_signal<T> r;

    SC_HAS_PROCESS(Target);

    explicit Target (sc_module_name name) : 
        sc_module(name) 
    {}

    void f(T val) {
        r = val;
    }
};

template<typename T>
struct AhbSlave : public sc_module, sc_interface
{
    sc_in_clk       clk;
    sc_in<bool>     nrst;

    sc_signal<T>    s;

    sc_vector<sc_port<port_if<T> > > slave_ports{"slave_ports", 2};
    
    SC_CTOR(AhbSlave) {
        SC_METHOD(methProc);
        sensitive << s;
    }
    
    void methProc() 
    {
        slave_ports[0]->f(3);
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;

    using T = sc_uint<4>;

    AhbSlave<T>             slave{"slave"};
    sc_vector< Target<T> >  tars{"tars", 2};
    
    SC_HAS_PROCESS(Dut);
    
    explicit Dut (sc_module_name name) : sc_module(name) 
    {
        slave.clk(clk);
        slave.nrst(nrst);
        slave.slave_ports[0](tars[1]);
        slave.slave_ports[1](tars[0]);
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
