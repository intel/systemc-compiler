/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by htrampur on 07/19/19
//

#include <systemc.h>

// Tests for pointer to MIF
struct A: public sc_module, sc_interface 
{
    sc_in_clk           clk;
    sc_signal<bool>     dummy;

    SC_CTOR(A){
        SC_METHOD(methProcA);
        sensitive << dummy;
    }
    
    void methProcA()
    {
        bool b = dummy;
    }
    
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<bool>     dummy;
    
    A*  a2;

    SC_CTOR(Dut)
    {
        a2 = new A("a2");
        a2->clk(clk);
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

