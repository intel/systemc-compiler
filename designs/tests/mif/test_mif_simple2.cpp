/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Simple MIF with one method process
struct mod_if: public sc_module, sc_interface 
{
    sc_in_clk           clk;
    sc_signal<bool>     s;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(mifProc);
        sensitive << s;
    }
    
    void mifProc()
    {
        bool b = s;
        s = !b;
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     s;
    
    mod_if*  minst;

    SC_CTOR(Dut)
    {
        minst = new mod_if("minst");
        minst->clk(clk);

        SC_METHOD(topProc);
        sensitive << minst->s;
    }
    
    void topProc()
    {
        bool b = minst->s;
        s = !b;
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

