/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Port promotion of various types of ports
SC_MODULE(inner) 
{
    sc_in_clk   clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    sc_in<sc_uint<8>>*      din;
    sc_out<sc_biguint<9>>   dout[3];

    SC_CTOR(inner) 
    {
        din = new sc_in<sc_uint<8>>("din");
                
        SC_CTHREAD(proc, clk.neg());
        async_reset_signal_is(rstn, false);
    }
    
    void proc() 
    {
        wait();
        
        while(1) {
            for (int i = 0; i < 3; i++) dout[i] = din->read() + i;
            wait();
        }
    }
    
};

SC_MODULE(top) {
    sc_in_clk   clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    inner inner0{"inner0"};

    SC_CTOR(top) {
        inner0.rstn(rstn);
    }
};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    sc_signal<sc_uint<8>>      din;
    sc_signal<sc_biguint<9>>   dout[3];

    sc_signal<bool> rstn;
    sc_clock clk{"clk", 1, SC_NS};
    top top0{"top0"};
    
    top0.clk(clk);
    top0.rstn(rstn);
    top0.inner0.clk(clk);

    top0.inner0.din->bind(din);
    for (int i = 0; i < 3; i++) {
        top0.inner0.dout[i](dout[i]);
    }
    sc_start();

    return 0;
}


