/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Check minimal FIFO size to full throughput
 */

#ifndef CHECK_SIZE_TEST_H
#define CHECK_SIZE_TEST_H

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
    sct_fifo<T, 2>      fifo{"fifo", 0, 1};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        //SC_METHOD(putMeth);
        SC_THREAD(putThread);
        sensitive << run << fifo.PUT;
        async_reset_signal_is(nrst, 0);

        SC_METHOD(getMeth);
        //SC_THREAD(getThread);
        sensitive << resp << fifo.GET;
        async_reset_signal_is(nrst, 0);
    }
    
    void putMeth() {
        run.reset_get();
        fifo.reset_put();
        
        if (run.request() && fifo.ready()) {
            T data = run.get();
            fifo.put(data);
            //cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into FIFO " << data << endl;
        }
    }
    
    void putThread() {
        run.reset_get();
        fifo.reset_put();
        wait();
        
        while (true) {
            if (run.request() && fifo.ready()) {
                T data = run.get();
                fifo.put(data);
                //cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into FIFO " << data << endl;
            }
            wait();
        }
    }
    
    void getMeth() {
        resp.reset_put();
        fifo.reset_get();
        
        if (fifo.request()) {
            assert (resp.ready());
            T data = fifo.get();
            resp.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from FIFO " << data << endl;
        }
    }
    
     void getThread() {
        resp.reset_put();
        fifo.reset_get();
        wait();
        
        while (true) {
            if (fifo.request()) {
                assert (resp.ready());
                T data = fifo.get();
                resp.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from FIFO " << data << endl;
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
    
    A<T>                a{"a"};

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
        
        SC_THREAD(put_thread);
        sensitive << run;
        async_reset_signal_is(nrst, 0);

        SC_THREAD(get_thread);
        sensitive << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    static const unsigned N = 20;
    
    void put_thread()
    {
        run.reset_put();
        wait();

        for (unsigned i = 0; i < N; ++i) {
            while (!run.put(42+i)) wait();
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, put done " << endl;
        wait();
        
        while (true) {
            wait();
        }
    }

    
    void get_thread()
    {
        sc_time ONE_NS = sc_time(1, SC_NS);
        resp.reset_get();
        wait();

        T data;
        sc_time t = sc_time_stamp();
        for (unsigned i = 0; i < N; ++i) {
            while (!resp.get(data, true)) wait();
            assert (data == 42+i);
        #ifndef SCT_TLM_MODE 
            assert (i == 0 || sc_time_stamp()-t == ONE_NS);
        #endif  
            t = sc_time_stamp();
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, get done = " << data << endl;
        wait();
        
        while (true) {
            wait();
        }
    }
};

#endif /* CHECK_SIZE_TEST_H */

