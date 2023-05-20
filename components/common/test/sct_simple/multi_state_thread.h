/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Multi-state THREAD test
 */

#ifndef MULTI_STATE_THREAD_H
#define MULTI_STATE_THREAD_H


#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

template<class T>
struct A : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp"};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        SC_THREAD(threadProc);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    
    std::array<T, 10> buf;
    void threadProc() {
        run.reset_get();
        resp.reset_put();
        wait();
        
        while(true) {
            unsigned j = 0;
            while (j < 3) {
                if (run.request()) {
                    buf[j] = run.get();
                    cout << sc_time_stamp() << " " << sc_delta_count() 
                         << " : Get from FIFO " << buf[j] << endl;
                    j++;
                }
                if (j != 3) wait(); // Do not have wait at last iteration
            }                       // to avoid sequential wait`s
            
            wait();

            j = 0;
            while (j < 3) {
                if (resp.ready()) {
                    resp.put(buf[j]);
                    j++;
                }
                wait();
            }
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
        cout << "multi_state_thread" << endl;
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
    
    const unsigned N = 3;
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        wait();

        while (!run.put(45)) wait();
        wait();
        while (!run.put(46)) wait();
        wait();
        while (!run.put(47)) wait();
        wait();

        while (!resp.get(data)) wait();
        assert (data == 45); wait();
        while (!resp.get(data)) wait();
        assert (data == 46); wait();
        while (!resp.get(data)) wait();
        assert (data == 47); wait();
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : All get done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* MULTI_STATE_THREAD_H */

