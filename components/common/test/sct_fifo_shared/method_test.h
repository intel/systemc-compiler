/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * FIFO used in two METHODs test
 */

#ifndef SHARED_METH_TEST_H
#define SHARED_METH_TEST_H

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
        SC_METHOD(putMeth);
        sensitive << run << fifo.PUT;

        SC_METHOD(getMeth);
        sensitive << resp << fifo.GET;
    }
    
    void putMeth() {
        run.reset_get();
        fifo.reset_put();
        
        if (run.request() && fifo.ready()) {
            T data = run.get();
            fifo.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into FIFO " << data << endl;
        } else {
            cout << sc_time_stamp() << " " << sc_delta_count() << " : NO put" << endl;
        }
    }
    
    void getMeth() {
        resp.reset_put();
        fifo.reset_get();
        
        if (fifo.request() && resp.ready()) {
            T data = fifo.get();
            //cout << sc_delta_count() << " : " << fifo.elem_num() << endl;
            resp.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from FIFO " << data << endl;
        } else {
            cout << sc_time_stamp() << " " << sc_delta_count() << " : NO get" << endl;
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
        cout << "fifo_shared meth" << endl;
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
    
    const unsigned N = 1;
    void init_thread()
    {
        run.reset_put();
        resp.reset_get();
        wait();

        for (unsigned i = 0; i < N; ++i) {
            while (!run.put(42+i)) wait();
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, put done " << endl;

        T data;
        for (unsigned i = 0; i < N; ++i) {
            while (!resp.get(data, true)) wait();
            assert (data == 42+i);
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, get done = " << data << endl;
        wait();
        
        while (true) {
            wait();
        }
    }
};

#endif /* SHARED_METH_TEST_H */

