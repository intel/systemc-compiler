/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Module with @sc_port to a MIF 2D vector element
template<typename T>
struct port_if : public sc_interface {
    virtual void f(T val) = 0;
    virtual T g(T val) = 0;
};

template<typename T>
struct Target : public sc_module, port_if<T> 
{
    T v;
    sc_signal<T> r;

    SC_HAS_PROCESS(Target);

    explicit Target (sc_module_name name) : 
        sc_module(name) 
    {}

    void f(T val) override {
        val = r.read();
    }

    T g(T val) override {
        r = val;
        return r.read();
    }
};

template<typename T>
struct AhbSlave : public sc_module, sc_interface 
{
    sc_in_clk       clk;
    sc_in<bool>     nrst;
    
    sc_signal<T>    s;

    sc_port<port_if<T> >  slave_port;
    
    SC_CTOR(AhbSlave) {
        SC_METHOD(methProc); sensitive << s;
    }
    
    void methProc()
    {
        slave_port->f(s.read());
        T l = slave_port->g(s.read());
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;

    using T = sc_uint<4>;

    AhbSlave<T>             slave{"slave"};
    sc_vector< sc_vector< Target<T>>>  tars{"tars", 2};
    
    SC_HAS_PROCESS(Dut);
    
    explicit Dut (sc_module_name name) : sc_module(name) 
    {
        slave.clk(clk);
        slave.nrst(nrst);
        tars[0].init(3);
        tars[1].init(3);
        
        slave.slave_port(tars[1][2]);
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
