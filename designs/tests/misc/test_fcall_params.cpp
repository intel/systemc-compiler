/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

using namespace sc_core;

// Bug from real design accelerators

namespace SinCos_ROM
{
    struct SinCosTuple
    {
        int sin;
        int cos;
    };
    
    const int sin_tab[] = {1, 2, 3, 4};
    const int cos_tab[] = {1, 2, 3, 4};

    SinCosTuple convert_sin_cos(unsigned idx)
    {
        unsigned new_idx = idx & 255;
        unsigned quadrant = idx / 256;

	int sin_val = sin_tab[new_idx];
	int cos_val = cos_tab[new_idx];

	SinCosTuple res;

	switch (quadrant)
	{
	case 0:
		res.cos = cos_val;
		res.sin = sin_val;
		break;
	case 1:
		res.cos = -sin_val;
		res.sin = cos_val;
		break;
	case 2:
		res.cos = -cos_val;
		res.sin = -sin_val;
		break;
	case 3:
		res.cos = sin_val;
		res.sin = -cos_val;
		break;
	}
        
        return res;
    }
};


template<int NCO_Width, int SinCosDataWidth>
struct A : public sc_module {
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<NCO_Width>>    nco_data;
    sc_out<sc_int<SinCosDataWidth> > sin_lane;
    sc_out<sc_int<SinCosDataWidth> > cos_lane;
    
    sc_in<bool> op_enable, next_op;
    sc_out<bool> op_enable_out, next_op_out;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_CTHREAD(fcall_param_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void fcall_param_thread() {
        using namespace SinCos_ROM;
        
        sin_lane.write(0);
        cos_lane.write(0);
        op_enable_out = 0;
        next_op_out = 0;
        
        while (true) {
            wait();
            
            op_enable_out = op_enable;
            next_op_out = next_op;
            
            if (op_enable.read()) 
            {
                SinCosTuple conv_res = convert_sin_cos(nco_data.read());
                sin_lane.write(conv_res.sin);
                cos_lane.write(conv_res.cos);
                
            } else {
                sin_lane.write(0);
                cos_lane.write(0);
            }
            
        }
    }    
};

class B : public sc_module {
public:
    sc_in<bool>         clk;

    sc_signal<bool> op_next[2];
    sc_signal<bool> op_en[2];
    sc_signal<sc_int<16> > sin_lane[2];
    sc_signal<sc_int<16> > cos_lane[2];
    
    A<10,16>* a_mod[2];
    
    B(const sc_module_name& name) : 
        sc_module(name)
    {
        for (int i = 0; i < 2; i++)
        {
            a_mod[i] = new A<10,16>("a");
            a_mod[i]->clk(clk);
            a_mod[i]->op_enable(op_en[i]);
            a_mod[i]->next_op(op_next[i]);
            a_mod[i]->op_enable_out(op_en[i]);
            a_mod[i]->next_op_out(op_next[i]);
            a_mod[i]->sin_lane(sin_lane[i]);
            a_mod[i]->cos_lane(cos_lane[i]);
            
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    B b_mod{"b_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

