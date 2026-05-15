/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

 //-----------------------------------------------------------------------------
// Fresh chan_demo example
// Pattern: one SC_METHOD producer + one SC_THREAD consumer with internal FIFO.
// Rules applied:
// - Explicit sensitivities contain only read channels/signals; no reset there.
// - Channels reset each METHOD invocation and in THREAD reset section only.
// - No waits in SC_METHOD; single wait per loop iteration in SC_THREAD.
//-----------------------------------------------------------------------------
#ifndef COPILOT_CHAN_DEMO_DEMO_H
#define COPILOT_CHAN_DEMO_DEMO_H

#include "sct_common.h"
#include <systemc.h>

namespace demo {

struct chan_demo : public sc_module {
    sc_in<bool> clk{"clk"};
    sc_in<bool> nrst{"nrst"};

    // External SS channel endpoints
    sct_target<int>    in_targ{"in_targ"};    // Testbench drives
    sct_initiator<int> out_init{"out_init"};  // Testbench monitors

    // Internal decoupling buffer (put in producer, get in consumer)
    sct_fifo<sc_int<16>,4> data_fifo{"data_fifo"};

    // Debug pulse when enqueue happened in current cycle
    sc_signal<bool> enq_ev{"enq_ev"};

    SC_HAS_PROCESS(chan_demo);

    chan_demo(sc_module_name n) : sc_module(n) {
        in_targ.clk_nrst(clk, nrst);
        out_init.clk_nrst(clk, nrst);
        data_fifo.clk_nrst(clk, nrst);

        // Producer METHOD: pull from target -> push into fifo
        SC_METHOD(producer_method);
        sensitive << in_targ << data_fifo.PUT; // no reset in list

        // Consumer THREAD: move fifo data -> out initiator
        SC_THREAD(consumer_thread);
        sensitive << data_fifo.GET << out_init; // explicit reads
        async_reset_signal_is(nrst, false);
    }

    // Combinational producer: attempt at most one transfer per activation.
    void producer_method() {
        // Reset channel sides used here each call per guideline
        in_targ.reset_get();
        data_fifo.reset_put();
        enq_ev = false;
        
        if (in_targ.request() && data_fifo.ready()) {
            int v = in_targ.get();
            data_fifo.put(v);
            enq_ev = true;
        }
    }

    void consumer_thread() {
        // Reset section
        data_fifo.reset_get();
        out_init.reset_put();
        wait();
        while (true) {
            if (data_fifo.request() && out_init.ready()) {
                int v = data_fifo.get();
                out_init.put(v);
            }
            wait();
        }
    }
};

} // namespace demo

#endif
