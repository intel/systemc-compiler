/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// wait(N) where N is unknown
SC_MODULE(test_mod) {

    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};

    SC_CTOR(test_mod) {
        SC_CTHREAD(wait_n_inf, clk);
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<unsigned> s;
    void wait_n_inf() 
    {
        unsigned n = s.read();
        wait();     

        while (1) {
            wait(n);
        }
    }
};


int sc_main(int argc, char **argv) {
    test_mod tmod{"tmod"};
    sc_start();
    return 0;
}
