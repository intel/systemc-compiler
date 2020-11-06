/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 1/29/18.
//

#include <systemc.h>

SC_MODULE(top)
{
    SC_CTOR(top)
    {
        clkin(clk);
        iport(isig);
        rstin(rstn);

        SC_THREAD(test_thread);
        sensitive << clk.posedge_event() << clk2.posedge_event();
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(test_cthread, clkin);
        async_reset_signal_is(rstn, false);
        reset_signal_is(rstin, false);

        SC_METHOD(test_method);
        sensitive << clkin.pos() << rstn.negedge_event();
        sensitive << clkin.neg() << clkin;
        sensitive << iport;
    }

    sc_clock clk{"clk", 10, SC_NS};
    sc_clock clk2{"clk2", 10, SC_NS};
    sc_in_clk clkin{"clkin"};
    sc_signal<bool> rstn{"rstn"};
    sc_in<bool> rstin{"rstin"};
    sc_signal<int> isig{"isig"};
    sc_in<int> iport{"iport"};

    void test_thread()
    {
        sc_stop();
    }

    void test_cthread() {}
    void test_method() {}
};

int sc_main(int argc, char **argv)
{
    top top0{"top0"};
    sc_start();
    return 0;
}

