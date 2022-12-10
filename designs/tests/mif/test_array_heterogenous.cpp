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

// Heterogenous array of module pointers, points to individual modules

template<typename T>
struct Target : public sc_module, sc_interface
{
    sc_signal<bool> r;

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

    Target<T>*      slaves[2];
    
    SC_CTOR(AhbSlave) {
        SC_METHOD(methProc);
        sensitive << clk.pos();
    }
    
    void methProc() 
    {
        for (int i = 0; i < 2; ++i) {
            slaves[i]->f(i);
        }
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;

    using T = sc_uint<4>;
    sc_signal<T>        s;
    
    AhbSlave<T>         slave{"slave"};
    Target<T>           tar1{"tar1"};
    Target<T>           tar2{"tar2"};
    
    SC_HAS_PROCESS(Dut);
    
    explicit Dut (sc_module_name name) : sc_module(name) 
    {
        slave.clk(clk);
        slave.nrst(nrst);
        slave.slaves[0] = &tar1;
        slave.slaves[1] = &tar2;
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
