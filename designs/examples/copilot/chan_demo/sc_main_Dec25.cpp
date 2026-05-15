/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

 //-----------------------------------------------------------------------------
// Fresh testbench for chan_demo
// - No reset in any sensitivity list (per guideline)
// - Threads have explicit async reset
//-----------------------------------------------------------------------------
#include "demo.h"
#include <systemc.h>

using namespace demo;
using namespace sct;

struct tb : public sc_module {
    sc_in<bool> clk{"clk"};
    sc_in<bool> nrst{"nrst"};

    chan_demo dut{"dut"};

    // Driver provides values to DUT target
    sct_initiator<int> drv_init{"drv_init"};
    // Monitor captures values from DUT initiator
    sct_target<int>    mon_targ{"mon_targ"};

    SC_HAS_PROCESS(tb);

    tb(sc_module_name n) : sc_module(n) {
        dut.clk(clk); dut.nrst(nrst);
        drv_init.clk_nrst(clk, nrst);
        mon_targ.clk_nrst(clk, nrst);

        drv_init.bind(dut.in_targ);
        mon_targ.bind(dut.out_init);

        SC_THREAD(driver_thread);
        sensitive << drv_init;  // reads drv_init.ready()
        async_reset_signal_is(nrst, false);

        SC_THREAD(monitor_thread);
        sensitive << mon_targ;  // reads mon_targ.request()
        async_reset_signal_is(nrst, false);
    }

    void driver_thread() {
        drv_init.reset_put();
        wait();
        int v = 0;
        while (true) {
            if (v < 16 && drv_init.ready()) {
                if (drv_init.put(v)) { v++; }
            }
            wait();
        }
    }

    void monitor_thread() {
        mon_targ.reset_get();
        wait();
        while (true) {
            if (mon_targ.request()) {
                int val = mon_targ.get();
                std::cout << sc_time_stamp() << " got " << val << std::endl;
            }
            wait();
        }
    }
};

SC_MODULE(top_env) {
    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<bool> nrst{"nrst"};
    tb tb_i{"tb_i"};

    SC_CTOR(top_env) {
        tb_i.clk(clk);
        tb_i.nrst(nrst);
    SC_THREAD(rst_thread);
    sensitive << clk; // wake each cycle to manage reset duration
    }

    void rst_thread() {
        nrst = false; wait(3); nrst = true; wait();
        wait(200);
        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {
    top_env env{"env"};
    sc_start();
    return 0;
}
