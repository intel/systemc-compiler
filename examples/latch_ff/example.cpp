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
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> enable{"enable"};
    sc_signal<sc_uint<16>> data_in{"data_in"};
    sc_signal<bool> rstn{"rstn"};
    sc_signal<sc_uint<16>> q_pos_out{"q_pos_out"};
    sc_signal<bool> arstn{"arstn"};
    sc_signal<sc_uint<16>>  async_rst_dff_out{"async_rst_dff_out"};


    Dut dut_inst{"dut_inst"};

    SC_CTOR(Tb) 
    {
        dut_inst.clk(clk);
        dut_inst.enable(enable);
        dut_inst.data_in(data_in);
        dut_inst.rstn(rstn);
        dut_inst.q_pos_out(q_pos_out);
        dut_inst.arstn(arstn);
        dut_inst.async_rst_dff_out(async_rst_dff_out);
    }
};
 
int sc_main (int argc, char **argv) 
{
    Tb tb("tb");
    sc_start();
    return 0;
}
