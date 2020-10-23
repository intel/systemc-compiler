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
    sc_signal<bool>         rstn{"rstn"};
    sc_signal<sc_uint<4> >  counter{"counter"};
    sc_signal<bool>         even{"even"};
 
    Dut dut_inst{"dut_inst"};

    SC_CTOR(Tb) 
    {
        dut_inst.clk(clk);
        dut_inst.rstn(rstn);
        dut_inst.counter(counter);
        dut_inst.even(even);
    }
};
 
int sc_main (int argc, char **argv) 
{
    Tb tb("tb");
    sc_start();
    return 0;
}