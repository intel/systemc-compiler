/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Verilog key words in local variables
SC_MODULE(library) {
    SC_CTOR(library) {
    }
};

SC_MODULE(A) {

    sc_in_clk clk;
    sc_signal <bool> rst;
    sc_signal <bool> wire;
    sc_out<sc_uint<4>> output;
    
    library l{"l"};
    
    SC_CTOR(A) {
        SC_METHOD(methodProc);
        sensitive << wire;
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rst, false);
    }    

    void methodProc() {
    	bool reg = wire.read();
    }

    void threadProc() {
        sc_uint<4> small = 0;
        output = 0;
    	wait();
        
        while (true) {
            output = small++;
            wait();
        }
    }
};


SC_MODULE(tb) {

    sc_clock clk {"clk", sc_time(1, SC_NS)};
    sc_signal<sc_uint<4>> output;

    A a{"a"};
    
    SC_CTOR(tb) {
        a.clk(clk);
        a.output(output);
    }

};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


