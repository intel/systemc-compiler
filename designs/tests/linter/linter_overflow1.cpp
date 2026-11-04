/******************************************************************************
 * Copyright (c) 2026, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Arithmetic overflow error detection in the linter. 
 */

#include "sct_common.h"
#include <systemc.h>

// Data type transferred through the channels
using Data = sct_uint<16>;

// Design under test
struct Dut : public sc_module
{
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};

    // Target to receive data, initiator to send data back
    sct_target<Data>        req{"req"};
    sct_initiator<Data>     resp{"resp"};

    SC_HAS_PROCESS(Dut);

    explicit Dut(const sc_module_name& name) : sc_module(name)
    {
        req.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        SC_THREAD(loopProc);
        sensitive << req << resp;
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
    }

    void loopProc()
    {
        Data data;
        req.reset_get();
        resp.reset_put();
        wait();

        while (true) {
            if (req.request() && resp.ready()) {
                data = req.get();

                // OVERFLOW: product of two 16-bit operands needs up to
                // 32 bits, but it is truncated to the 16-bit type Data.
                Data square = data * data;

                resp.put(square);
            }
            wait();
        }
    }
};

// Testbench
struct Tb : public sc_module
{
    sc_in<bool>             clk{"clk"};
    sc_signal<bool>         nrst{"nrst"};

    // Initiator to drive the DUT, target to collect the response
    sct_initiator<Data>     req{"req"};
    sct_target<Data>        resp{"resp"};

    Dut                     dut{"dut"};

    SC_HAS_PROCESS(Tb);

    explicit Tb(const sc_module_name& name) : sc_module(name)
    {
        req.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        dut.clk(clk);
        dut.nrst(nrst);
        dut.req.bind(req);
        dut.resp.bind(resp);

        SC_THREAD(driveProc);
        sensitive << req << resp;
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);

        SCT_CTHREAD(resetProc, clk, SCT_CMN_TRAITS::CLOCK);
    }

    static const unsigned N = 8;

    void driveProc()
    {
        Data data;
        req.reset_put();
        resp.reset_get();
        wait();

        for (unsigned i = 0; i < N; ++i) {
            Data sent = i + 1;
            while (!req.put(sent)) wait();
            wait();
            while (!resp.get(data)) wait();
            wait();
        }

        cout << sc_time_stamp() << " : all data looped back" << endl;

        while (true) {
            wait();
        }
    }

    void resetProc()
    {
        nrst = SCT_CMN_TRAITS::RESET;
        wait();
        nrst = !SCT_CMN_TRAITS::RESET;
        wait(200);

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
    Tb tb{"tb"};
    tb.clk(clk);
    sc_start();
    return 0;
}
