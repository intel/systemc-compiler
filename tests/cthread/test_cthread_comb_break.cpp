/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/22/18.
//

#include <systemc.h>

class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int>  out{"out"};
    sc_signal<int>  in{"in"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(test_thread);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    void test_thread()
    {
        out = 0;
        while (1) {
            wait();

            if (in.read())
                wait();

//            for (size_t i = 0; i < 3; ++i) {
//
//                if (in.read())
//                    break;
//
//                wait();
//            }
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

