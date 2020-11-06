/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Simple module binds to each other example

SC_MODULE(Producer) {
    sc_signal<bool>         req_sig{"req_sig"};
    sc_out<bool>            resp{"resp"};
    sc_out<sc_int<16>>      data{"data"};

    SC_CTOR(Producer) {}
};

SC_MODULE(Consumer) {
    sc_in<bool>             req{"req"};
    sc_signal<bool>         resp_sig{"resp_sig"};
    sc_in<sc_int<16>>       data{"data"};
    
    SC_CTOR(Consumer) {}
};

SC_MODULE(Parent) {
    Producer prod{"prod"};
    Consumer cons{"cons"};
    sc_signal<sc_int<16>>   data{"data"};

    SC_CTOR(Parent) {
        cons.req(prod.req_sig);     // in-to-signal
        prod.resp(cons.resp_sig);   // out-to-signal
        
        prod.data(data);            // in-to-signal-to-out
        cons.data(data);
    }
};


SC_MODULE(tb) {

    Parent top_mod{"top_mod"};

    SC_CTOR(tb) {
    }

};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


