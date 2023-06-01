/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Simple THREAD test blocking put and get in initiator/target/FIFO
 */

#ifndef SIMPLE_THREAD_TEST_H
#define SIMPLE_THREAD_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

template<class T>
struct A : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp", 1};
    sct_fifo<T,2>       fifo{"fifo"};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        fifo.clk_nrst(clk, nrst);

        SC_THREAD(runProc);
        sensitive << run << fifo.PUT;
        async_reset_signal_is(nrst, 0);
        
        SC_THREAD(respProc);
        sensitive << resp << fifo.GET;
        async_reset_signal_is(nrst, 0);
    }
    
    void runProc() {
        run.reset_get();
        fifo.reset_put();
        wait();
        
        while(true) {
            T data = run.b_get();
            cout << sc_time_stamp() << " " << sc_delta_count() 
                 << " : Put to FIFO " << data << endl;
            fifo.b_put(data);
//                cout << sc_time_stamp() << " " << sc_delta_count() 
//                     << " : put done " << endl;
            wait();
        }
    }
    
    void respProc() {
        fifo.reset_get();
        resp.reset_put();
        wait();

        while(true) {
            T data = fifo.b_get();
            cout << sc_time_stamp() << " " << sc_delta_count() 
                 << " : Get from FIFO " << fifo.get() << endl;
            resp.b_put(data);
            wait();
        }
    }
};


class simple_test : public sc_module 
{
public:
    using T = sc_uint<16>;

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_initiator<T>    run{"run"};
    sct_target<T>       resp{"resp"};

    A<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
        SC_THREAD(init_thread);
        run.addTo(sensitive);
        resp.addTo(sensitive);
        async_reset_signal_is(nrst, 0);
    }
    
    const unsigned N = 6;
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        wait();

        // Scenario #1. Multiple push w/o pop
        for (int i = 0; i < N; ++i) {
            while (!run.put(32+i)) wait();
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1, all put done " << endl;

        for (int i = 0; i < N; ++i) {
            while (!resp.get(data)) wait();
            assert (data == 32+i);
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1 all get done " << endl << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_THREAD_TEST_H */

