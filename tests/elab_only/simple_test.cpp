/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 1/26/18.
//

#include <systemc.h>

SC_MODULE(submodule)
{
    sc_in_clk   clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    SC_CTOR(submodule) {
        SC_METHOD(test_cmethod);
        sensitive << clk.pos();
    }

    void test_cmethod() {
#ifdef __SC_TOOL__
        cout << "RUNNING IN SC_TOOL\n";
#endif

#ifdef __SC_TOOL_ANALYZE__
        cout << "RUNNING IN SC_TOOL ANALYZER\n";
#endif

    }
};


SC_MODULE(top) {
    SC_CTOR(top) {
        SC_THREAD(test_thread);
        sensitive << clk.posedge_event();

        smod.clk(clk);
        smod.rstn(rstn);
    }

private:

    void test_thread() {
        wait(1,SC_NS);
        cout << "Done at " << sc_time_stamp() << "\n";
        sc_stop();
    }

    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};

    submodule smod{"smod"};
};

int sc_main(int argc, char **argv) {
    auto top_inst = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}
