/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Simple port promotion example, used for UG
SC_MODULE(Child) {
    sc_in_clk   clk{"clk"};

    sc_in<sc_int<16>>  in{"in"};
    sc_out<sc_int<16>> out{"out"};

    SC_CTOR(Child) {
        SC_METHOD(proc); sensitive << in;
    }
    
    void proc() {
        out = in.read() + 1;
    }
};

SC_MODULE(Top) {
    sc_in_clk   clk{"clk"};
    Child child_inst{"child_inst"};

    SC_CTOR(Top) {
        child_inst.clk(clk);
    }
};


SC_MODULE(tb) {

    Top top_mod{"top_mod"};

    sc_signal<bool>         clk{"clk"};
    sc_signal<sc_int<16>>   in{"in"};
    sc_signal<sc_int<16>>   out{"out"};

    SC_CTOR(tb) {
        top_mod.clk(clk);
        top_mod.child_inst.in(in);
        top_mod.child_inst.out(out);
    }

};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


