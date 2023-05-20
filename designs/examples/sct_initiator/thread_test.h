/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Simple THREAD test with SYNC option in initiator
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
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        SC_THREAD(threadProc);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    void threadProc() {
        run.reset_get();
        resp.reset_put();
        wait();
        
        while(true) {
            resp.clear_put();
            if (run.request() && resp.ready()) {
                T data = run.get();
                resp.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() 
                     << " : Get " << data << endl;
            }
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
    
    const unsigned N = 3;
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        wait();

        while (!run.put(42)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : Put 42" << endl;
        wait();
        
        while (!resp.get(data)) wait();
        assert (data == 42); wait();
        
        while (!run.put(43)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : Put 43" << endl;
        wait();
        while (!run.put(44)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : Put 44" << endl;
        wait();
        
        while (!resp.get(data)) wait();
        assert (data == 43); wait();
        while (!resp.get(data)) wait();
        assert (data == 44); wait();

        while (!run.put(45)) wait();
        wait();
        while (!run.put(46)) wait();
        wait();
        while (!run.put(47)) wait();
        wait();
        while (!run.put(48)) wait();
        wait();

        while (!resp.get(data)) wait();
        assert (data == 45); wait();
        while (!resp.get(data)) wait();
        assert (data == 46); wait();
        while (!resp.get(data)) wait();
        assert (data == 47); wait();
        while (!resp.get(data)) wait();
        assert (data == 48); wait();
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #2 all get done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_THREAD_TEST_H */

