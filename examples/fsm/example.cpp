/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "dut.h"
#include <systemc.h>
 
// SVC requires DUT top should be instantiated inside wrapper (typically TB) 
// and all DUT ports are bound.
struct Tb : sc_module 
{
    sc_clock                clk{"clk", sc_time(1, SC_NS)};
    sc_signal<bool>         rst{"rst"};
    sc_signal<sc_uint<1>>   a;
    sc_signal<sc_uint<1>>   zs;
    sc_signal<sc_uint<1>>   zv;

    Dut dut_inst{"dut_inst"};

    SC_CTOR(Tb) 
    {
        dut_inst.clk(clk);
        dut_inst.rst(rst);
        dut_inst.a(a);
        dut_inst.zs(zs);
        dut_inst.zv(zv);
    }
};
 
int sc_main (int argc, char **argv) 
{
    Tb tb("tb");
    sc_start();
    return 0;
}
