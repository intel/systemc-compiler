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
    sc_in_clk               clk{"clk"};
    sc_signal<bool>         rstn{"rstn"};
    sc_signal<sc_uint<4> >  counter{"counter"};
    sc_signal<bool>         even{"even"};
 
    Dut dut_inst{"dut_inst"};
    
    SC_CTOR(Tb) 
    {
        SC_CTHREAD(reset_test, clk.pos());
        
        dut_inst.clk(clk);
        dut_inst.rstn(rstn);
        dut_inst.counter(counter);
        dut_inst.even(even);
        
        SC_METHOD(print_method);
        sensitive << clk.pos();
    }

    void print_method() {
        cout << sc_time_stamp() << "\t\t" << rstn.read() << "\t\t" 
             << counter.read() << "\t\t" << even.read() << endl;
    }

    void reset_test() {
        rstn = 1;
        wait(2);
        
        rstn = 0;
        wait(3);
        
        rstn = 1;
        wait(100);
    }

};
 
int sc_main (int argc, char **argv) 
{
    sc_clock clk{"clk", sc_time(1, SC_NS)};
    Tb tb("tb");
    tb.clk(clk);
    
    cout <<"Time\t\tReset\t\tCounter\t\tEven" << endl;
    sc_start(20, SC_NS);
    cout <<"\n\t\t*****Simulation complete*****" << endl;

    return 0;
}