/******************************************************************************
 * Copyright (c) 2026, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Incorrect inter-process synchronization via raw signals.
 */

#include "sct_common.h"
#include <systemc.h>

using Data = sct_uint<16>;

struct Dut : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_signal<bool>     request{"request"};
    sc_signal<bool>     response{"response"};
    sc_signal<Data>     payload{"payload"};
    
    SC_HAS_PROCESS(Dut);

    explicit Dut(const sc_module_name& name) : sc_module(name)
    {
        SC_THREAD(producerProc);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);

        SC_THREAD(consumerProc);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
    }

    void producerProc()
    {
        Data count = 1;
        request = false;
        payload = 0;
        wait();

        while (true) {            
            if (!request.read() && !response.read()) {
                payload = count;
                request = true;
                count = count + 1;
            } else if (request.read() && response.read()) {
                request = false;
            }
            wait();
        }
    }

    void consumerProc()
    {
        response = false;
        Data received = 0;
        wait();

        while (true) {
            Data value = payload.read();
            bool accept = value.range(2, 0) != 0;
            if (request.read() && !response.read() && accept) {
                received = payload.read();
                response = true;
            } else if (!request.read() && response.read()) {
                response = false;
            }
            wait();
        }
    }
};

struct Tb : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    Dut                 dut{"dut"};

    SC_HAS_PROCESS(Tb);

    explicit Tb(const sc_module_name& name) : sc_module(name)
    {
        dut.clk(clk);
        dut.nrst(nrst);

        SC_CTHREAD(resetProc, clk.pos());
    }

    void resetProc()
    {
        nrst = SCT_CMN_TRAITS::RESET;
        wait();
        nrst = !SCT_CMN_TRAITS::RESET;
        wait(50);
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