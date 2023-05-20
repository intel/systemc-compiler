/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Check @bind_rtl() method of target/initiator
 */

#include "sct_common.h"
#include <systemc.h>

class simple_test : public sc_module 
{
public:
    using T = sc_uint<16>;

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_in<bool>         run_core_req{"run_core_req"};
    sc_in<T>            run_core_data{"run_core_data"};
    sc_out<bool>        run_core_ready{"run_core_ready"};
    
    sc_out<bool>        resp_core_req{"resp_core_req"};
    sc_out<T>           resp_core_data{"resp_core_data"};
    sc_in<bool>         resp_core_ready{"resp_core_ready"};

    SC_HAS_PROCESS(simple_test);
    
    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {}
};

class Test_top : public sc_module
{
public:
    using T = sc_uint<16>;

    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    // This test intended for RTL mode only
#ifndef SCT_TLM_MODE
    sct_initiator<T>    SC_NAMED(run);
    sct_target<T>       SC_NAMED(resp);
    simple_test         SC_NAMED(dut);
#endif

    SC_CTOR(Test_top) {
    #ifndef SCT_TLM_MODE
        dut.clk(clk);
        dut.nrst(nrst);
        run.clk_nrst(clk, nrst); 
        resp.clk_nrst(clk, nrst);
        
        SCT_BIND_CHANNEL(dut, run, run);
        SCT_BIND_CHANNEL(dut, resp, resp);

        SCT_CTHREAD(testProc, clk, SCT_CMN_TRAITS::CLOCK);
        sensitive << run << resp;
    #endif
    }
    
    void testProc() {
        nrst = 1;
        wait();
        nrst = 0;
        wait(10);

        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    sct_clock<> clk{"clk", 1, SC_NS};
    Test_top test_top{"test_top"};
    test_top.clk(clk);
    sc_start();
    return 0;
}
