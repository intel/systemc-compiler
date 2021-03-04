/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Port promotion through multiple modules
SC_MODULE(bottom) {
    sc_in_clk   clkin{"clkin"};

    sc_in<bool> din{"din"};

    SC_CTOR(bottom) {
    }
    
};


SC_MODULE(bottom2) {
    sc_in_clk   clkin{"clkin"};

    sc_in<sc_uint<12>> din{"din"};
    sc_out<int> dout{"dout"};

    sc_in<int> intin{"intin"};

    SC_CTOR(bottom2) {
    }
    
};

SC_MODULE(inner) {

    sc_in_clk   clkin{"clkin"};
    sc_in<bool> rstn{"rstn"};

    sc_in<sc_int<12>> din{"din"};
    sc_out<sc_int<12>> inner_out{"inner_out"};

    bottom bot0{"bot0"};
    bottom2 bot2{"bot2"};

    sc_signal<int> intsig{"intsig"};


    SC_CTOR(inner) {
        bot0.clkin(clkin);
        bot2.clkin(clkin);
        bot2.intin(intsig);
    }
    
};

SC_MODULE(top) {
    sc_in<bool> rstn{"rstn"};

    inner inner0{"inner0"};

    SC_CTOR(top) {
        inner0.rstn(rstn);
    }
    
};


SC_MODULE(tb) {

    top top0{"top0"};

    sc_signal <bool> din{"din"};
    sc_signal <sc_uint<12>> din2{"din2"};

    sc_signal <bool> clkgen{"clkgen"};
    sc_signal <int> dout{"dout"};

    sc_signal<bool> rstn{"rstn"};

    sc_signal<sc_int<12>> din3{"din3"};
    sc_signal<sc_int<12>> inner_out{"inner_out"};


    SC_CTOR(tb) {
        top0.rstn(rstn);

        top0.inner0.din(din3);
        top0.inner0.inner_out(inner_out);

        top0.inner0.clkin(clkgen);
        top0.inner0.bot0.din(din);

        top0.inner0.bot2.din(din2);
        top0.inner0.bot2.dout(dout);
    }

};



int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


