/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

// ICSC requires DUT top should be instantiated inside wrapper (typically TB) 
// and all DUT ports are bound.

#include "dut.h"
#include <systemc.h>
 
struct Tb : sc_module 
{
    sc_signal<bool> enable{"enable"};
    sc_signal<sc_uint<4>> data_in{"data_in"};
    sc_signal<sc_uint<2>> bdata_in{"bdata_in"};

    sc_signal<sc_uint<2>> binary_dout{"binary_dout"};
    sc_signal<sc_uint<2>> binary_case_dout{"binary_case_dout"};
    sc_signal<sc_uint<2>> binary_prior_dout{"binary_prior_dout"};
    sc_signal<sc_uint<2>> binary_prior_assign_dout{"binary_prior_assign_dout"};
    sc_signal<sc_uint<4>> binary_decoder_case_dout{"binary_decoder_case_dout"};
    sc_signal<sc_uint<4>> binary_decoder_assign_dout{"binary_decoder_assign_dout"};


    Dut dut_inst{"dut_inst"};

    SC_CTOR(Tb) 
    {
        dut_inst.enable(enable);
        dut_inst.data_in(data_in);
        dut_inst.bdata_in(bdata_in);
        dut_inst.binary_dout(binary_dout);
        dut_inst.binary_case_dout(binary_case_dout);
        dut_inst.binary_prior_dout(binary_prior_dout);
        dut_inst.binary_prior_assign_dout(binary_prior_assign_dout);
        dut_inst.binary_decoder_case_dout(binary_decoder_case_dout);
        dut_inst.binary_decoder_assign_dout(binary_decoder_assign_dout);
    }
};
 
int sc_main (int argc, char **argv) 
{
    Tb tb("tb");
    sc_start();
    return 0;
}
