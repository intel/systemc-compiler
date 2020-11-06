/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/27/18.
//

#include <systemc.h>

struct write_if : sc_interface {
    virtual void write(int val) = 0;
};

struct top : sc_module, write_if {

    sc_clock    clk_gen{"clk_gen", 10, SC_NS};
    sc_signal <bool> arstn_sig{"arstn_sig", 1};


    sc_in<bool> clk{"clk"};
    sc_in<bool> arstn{"arstn"};

    sc_port<write_if> write_port{"write_port"};

    SC_CTOR(top) {

        clk(clk_gen);
        arstn(arstn_sig);
        write_port.bind(*this);

        SC_CTHREAD(top_thread, clk);
        async_reset_signal_is(arstn, false);
    }


    void top_thread() {
        wait();
        write(12);
        wait();
        sc_stop();
    }

    void write(int val) override {
        cout << "write " << val << endl;
    }


};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

