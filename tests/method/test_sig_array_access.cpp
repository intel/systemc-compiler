/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Channel array access including function call
struct dut : sc_module {

    sc_in_clk   clk{"clk"};

    sc_in<bool> en{"en"};
    sc_in<bool> r_nw{"r_nw"};
    sc_in<sc_uint<2>>  addr{"addr"};

    sc_in<sc_uint<32>>  wdata{"wdata"};
    sc_out<sc_uint<32>> rdata{"rdata"};

    SC_CTOR(dut) {
        SC_METHOD(write_method_ff);
        sensitive << en << r_nw << addr << wdata;

        SC_METHOD(read_method_comb);
        sensitive << en << r_nw << addr << reg_file[0];

        SC_METHOD(chan_arr_func_param);  
        sensitive << chr[0] << chr[1] << chr[2];

        SC_METHOD(chan_arr_func_param2);  
        sensitive << chp[2][1];
    }

private:

    sc_signal<sc_uint<32>> reg_file[4];

    void write_method_ff() {
        if (en && !r_nw)
            reg_file[addr.read()] = wdata;
    }

    void read_method_comb() {
        rdata = 0;
        if (en && r_nw)
            rdata = reg_file[addr.read()];
    }

//---------------------------------------------------------------------------    
    
    void chan_func(bool par) {
        bool c = par;
    }
    
    sc_signal<bool> chr[3];
    
    void chan_arr_func_param() 
    {
        chan_func(chr[1].read());
    }
    
    
    sc_signal<bool> chp[3][2];
    
    void chan_arr_func_param2() 
    {
        chan_func(chp[2][1].read());
    }
};

struct tb : sc_module {

    sc_clock clk{"clk",10,SC_NS};

    sc_signal <bool>         en{"en"};
    sc_signal <bool>         r_nw{"r_nw"};
    sc_signal <sc_uint<2>>   addr{"addr"};
    sc_signal <sc_uint<32>>  wdata{"wdata"};
    sc_signal <sc_uint<32>>  rdata{"rdata"};

    dut dut_inst{"dut_inst"};

    SC_CTOR(tb) {

        dut_inst.clk(clk);
        dut_inst.en(en);
        dut_inst.r_nw(r_nw);
        dut_inst.addr(addr);
        dut_inst.wdata(wdata);
        dut_inst.rdata(rdata);
    }

};


int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk",10,SC_NS};

    sc_signal <bool>         en{"en"};
    sc_signal <bool>         r_nw{"r_nw"};
    sc_signal <sc_uint<2>>   addr{"addr"};
    sc_signal <sc_uint<32>>  wdata{"wdata"};
    sc_signal <sc_uint<32>>  rdata{"rdata"};
    
    dut dut_inst{"dut_inst"};
    dut_inst.clk(clk);
    dut_inst.en(en);
    dut_inst.r_nw(r_nw);
    dut_inst.addr(addr);
    dut_inst.wdata(wdata);
    dut_inst.rdata(rdata);

    sc_start();
    return 0;
}
