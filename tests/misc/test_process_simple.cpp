/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Simple processes example

SC_MODULE(MyModule) {
    sc_in<bool>     in{"in"};
    sc_signal<int>  sig{"sig"};
    sc_out<bool>    out{"out"};
    SC_CTOR(MyModule) {
        SC_METHOD(methodProc);
        sensitive << in << sig;
    }    
    void methodProc() {
    	bool b = in;   // Use in, it need to be in sensitive list
        if (sig != 0) {     // Use sig, it need to be in sensitive list
    	    out = b;
        } else {
            out = 0;
        }
    }
};


SC_MODULE(tb) {

    MyModule top_mod{"top_mod"};

    sc_signal<bool> s1;
    
    SC_CTOR(tb) {
        top_mod.in(s1);
        top_mod.out(s1);
    }

};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


