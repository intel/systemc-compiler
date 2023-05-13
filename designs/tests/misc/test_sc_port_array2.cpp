/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Module with @sc_port to a MIF vector element, used in method and thread
template<typename T>
struct port_if : public sc_interface {
    virtual void f(T val) = 0;
    virtual void fa(T val) = 0;
    virtual void g(T val) = 0;
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
        T l = val;
        v = l + 1;
        val = r.read();
    }

    T a[2];
    void fa(T val) override {
        T l[2];
        l[0] = val;
        l[1] = r.read();
        int i = r.read();
        a[i] = l[i];
    }
    
    void g(T val) override {
        r = val;
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
        
        SC_CTHREAD(thrdProc1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thrdProc2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void methProc()
    {
        slave_port->f(s.read());
        slave_port->fa(s.read());
    }

    void thrdProc1()
    {
        slave_port->f(1);
        wait();
        while (true) {
            slave_port->fa(s.read());
            wait();
        }
    }

    void thrdProc2()
    {
        wait();
        while (true) {
            slave_port->g(s.read());
            wait();
        }
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
        slave.slave_port(tars[1]);
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
