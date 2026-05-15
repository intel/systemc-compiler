/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#ifndef COPILOT_CHAN_DEMO_DEMO_H
#define COPILOT_CHAN_DEMO_DEMO_H

#include "sct_common.h"
#include <systemc.h>

namespace demo {

struct chan_demo : public sc_module {
    static const unsigned FIFO_DEPTH = 4;

    sc_in_clk clk{"clk"};
    sc_in<bool> nrst{"nrst"};

    sct_target<int> in_targ{"in_targ"};
    sct_initiator<int> out_init{"out_init"};
    sct_fifo<int, FIFO_DEPTH> data_fifo{"data_fifo"};

    SC_HAS_PROCESS(chan_demo);

    explicit chan_demo(const sc_module_name& name) : sc_module(name) {
        in_targ.clk_nrst(clk, nrst);
        out_init.clk_nrst(clk, nrst);
        data_fifo.clk_nrst(clk, nrst);

        SCT_METHOD(producer_method);
        in_targ.addTo(sensitive);
        data_fifo.addToPut(sensitive);

        SCT_THREAD(consumer_thread, clk, nrst);
        data_fifo.addToGet(sensitive);
        out_init.addTo(sensitive);
        async_reset_signal_is(nrst, false);
    }

    void producer_method() {
        in_targ.reset_get();
        data_fifo.reset_put();

        if (in_targ.request() && data_fifo.ready()) {
            int data = in_targ.get();
            data_fifo.put(data);
        }
    }

    void consumer_thread() {
        int data;

        data_fifo.reset_get();
        out_init.reset_put();
        data = 0;
        wait();

        while (true) {
            out_init.clear_put();

            if (data_fifo.request() && out_init.ready()) {
                data = data_fifo.get();
                out_init.put(data);
            }

            wait();
        }
    }
};

} // namespace demo

#endif // COPILOT_CHAN_DEMO_DEMO_H