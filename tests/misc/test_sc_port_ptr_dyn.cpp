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

// Module with @sc_port and pointer to dynamically allocated module
// Taken from real design accelerators

template<typename T>
struct port_if : public sc_interface {
    virtual T f(T val) = 0;
    virtual void g(T val) = 0;
    virtual T h() = 0;
};

template<typename T>
struct AhbSlave : public sc_module, sc_interface
{
    sc_in_clk       clk;
    sc_in<bool>     nrst;

    port_if<T>*     slave_port;
    
    SC_CTOR(AhbSlave) {
        SC_METHOD(methProc);
        sensitive << clk.pos();

        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(nrst, false);
    }
    
    void methProc() 
    {
        slave_port->g(0);        
    }
     
    void threadProc() 
    {
        T a = 0;
        slave_port->f(0);
        wait();
        
        while (true) {
            a = slave_port->f(a);
            wait();
        }
    }
};

struct DutBase : public sc_module {
    SC_CTOR(DutBase) 
    {}
};

struct Dut : public DutBase 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;

    using T = sc_uint<4>;
    sc_signal<T>        s;
    
    template<typename T>
    struct Target : public sc_module, port_if<T> 
    {
        Dut* parent;
        T l1;
        T l2;
        sc_signal<T> s2;

        explicit Target (sc_module_name name, Dut* parent_) : 
            sc_module(name), 
            parent(parent_) 
        {}

        T f(T val) {
            parent->s = val;
            return (val+1);
        }
        
        void g(T val) {
            l1 = val;
            s2 = l1 + 1;
        } 
        
        T h() {
            return (l2 + s2.read());
        } 
    };

    AhbSlave<T>         slave{"slave"};
    Target<T>*          ptar;
    
    SC_HAS_PROCESS(Dut);
    
    explicit Dut (sc_module_name name) : DutBase(name) 
    {
        ptar = new Target<T>("ptar", this);

        slave.clk(clk);
        slave.nrst(nrst);
        slave.slave_port = ptar;

        SC_METHOD(methProc);
        sensitive << ptar->s2;
    }
    
    void methProc() 
    {
        T a = ptar->h();        
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
