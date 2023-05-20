/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * FIFO used inside of THREAD/METHOD tests, 
 * Modules A and B are two options to have get/put in METHOD/THREAD
 */

#ifndef SHARED_THREAD_TEST_H
#define SHARED_THREAD_TEST_H

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
    sct_fifo<T, 2>      fifo{"fifo", 1, 0};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(putMeth);
        sensitive << run << fifo.PUT;
        async_reset_signal_is(nrst, 0);

        SC_THREAD(getThread);
        sensitive << resp << fifo.GET;
        async_reset_signal_is(nrst, 0);
    }
    
    void putMeth() {
        run.reset_get();
        fifo.reset_put();
        
        if (run.request() && fifo.ready()) {
            T data = run.get();
            fifo.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into FIFO " << data << endl;
        }
    }
    
    void getThread() {
        resp.reset_put();
        fifo.reset_get();
        wait();
        
        while(true) {
            if (fifo.request() && resp.ready()) {
                T data = fifo.get();
                resp.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from FIFO " << data << endl;
            }
            wait();
        }
    }
};

template<class T>
struct B : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp"};
    sct_fifo<T, 2>      fifo{"fifo", 0, 1};
  
    SC_HAS_PROCESS(B);
    
    explicit B(const sc_module_name& name) : sc_module(name)
    {
        SC_THREAD(putThread);
        sensitive << run << fifo;
        async_reset_signal_is(nrst, 0);

        SC_METHOD(getMeth);
        sensitive << resp << fifo;
        async_reset_signal_is(nrst, 0);
    }
    
    void putThread() {
        run.reset_get();
        fifo.reset_put();
        wait();
        
        while(true) {
            if (run.request() && fifo.ready()) {
                T data = run.get();
                fifo.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into FIFO " << data << endl;
            }
            wait();
        }
    }

    void getMeth() {
        resp.reset_put();
        fifo.reset_get();
        
        if (fifo.request() && resp.ready()) {
            T data = fifo.get();
            resp.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from FIFO " << data << endl;
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
    
    A<T>    a{"a"};
    //B<T>    a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.clk_nrst(clk, nrst);
        a.resp.clk_nrst(clk, nrst);
        a.fifo.clk_nrst(clk, nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
        SC_THREAD(init_thread);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    const unsigned N = 5;
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
        
        // Scenario #2. Push mixed with pop
        while (!run.put(42)) wait();
        wait();
        
        while (!resp.get(data)) wait();
        assert (data == 42); wait();
        
        while (!run.put(43)) wait();
        wait();
        while (!run.put(44)) wait();
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

        while (!resp.get(data)) wait();
        assert (data == 45); wait();
        while (!resp.get(data)) wait();
        assert (data == 46); wait();
        while (!resp.get(data)) wait();
        assert (data == 47); wait();
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #2 all get done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SHARED_THREAD_TEST_H */

