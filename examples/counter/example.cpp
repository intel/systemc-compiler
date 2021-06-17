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
    
    void print_method(){
        cout <<clk.read()<<"\t\t"<<sc_time_stamp()<<"\t\t"<<rstn.read()<<"\t\t"<<counter.read()<<"\t\t"<<even.read()<<endl;
    }

    void reset_test(){
            rstn.write(1);
            wait(2, SC_NS);
            rstn.write(0);
            wait(3, SC_NS);
            rstn.write(1);
            wait();
    }

    SC_CTOR(Tb) 
    {
        SC_THREAD(reset_test);
        sensitive << rstn;
        dut_inst.clk(clk);
        dut_inst.rstn(rstn);
        dut_inst.counter(counter);
        dut_inst.even(even);
        SC_METHOD(print_method);
        sensitive << counter;
    }
};
 
int sc_main (int argc, char **argv) 
{
    Tb tb("tb");
    cout <<"Clock\t\tTime\t\tReset\t\tCounter\t\tEven" << endl;
    sc_start(22, SC_NS);
    cout <<"\n\t\t*****Simulation complete*****" << endl;
    return 0;
}
