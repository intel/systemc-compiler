/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 06/27/19s.
//

#include <systemc.h>

// Module with array of @sc_port to module object on stack
template<typename T>
struct port_if : public sc_interface {
    virtual void f(T val) = 0;
};

template<typename T>
struct AhbSlave : public sc_module
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
        
        for (int i = 0; i < 2; ++i) {
            slave_ports[i]->f(i);
        }
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;

    using T = sc_uint<4>;

    template<typename T>
    struct Target : public sc_module, port_if<T> 
    {
        sc_signal<T> r;

        explicit Target (sc_module_name name) : 
            sc_module(name) 
        {}

        void f(T val) {
            r = val;
        }
    };

    AhbSlave<T>         slave{"slave"};
    Target<T>*          tars[2];
    
    SC_HAS_PROCESS(Dut);
    
    explicit Dut (sc_module_name name) : sc_module(name) 
    {
        slave.clk(clk);
        slave.nrst(nrst);

        for (int i = 0; i < 2; ++i) {
            tars[i] = new Target<T>("tar");
        }
        for (int i = 0; i < 2; ++i) {
            slave.slave_ports[i](*tars[i]);
        }
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
