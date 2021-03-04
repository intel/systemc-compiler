/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// wait(N) where N is not constant, changed at next loop iteration, failed
SC_MODULE(test_mod) {

    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};

    SC_CTOR(test_mod) {
        SC_CTHREAD(wait_n_inf, clk);
        async_reset_signal_is(rstn, false);
    }
    
    void wait_n_inf() 
    {
        sc_uint<10> n = 1;
        wait();     

        while (1) {
            wait(n);
            n++;
        }
    }
};


int sc_main(int argc, char **argv) {
    test_mod tmod{"tmod"};
    sc_start();
    return 0;
}
