/******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * PIPE used inside of single THREAD test
 */

#ifndef SINGLE_TEST_H
#define SINGLE_TEST_H

#include "sct_pipe.h"
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
    sct_pipe<T, 3>      pipe{"pipe", 0, 0, "My_SV_pipe_reg"};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        cout << "pipe single" << endl;
        pipe.clk_nrst(clk, nrst);
        
        SCT_METHOD(methProc);
        sensitive << run << resp << pipe;
    }
    
    void methProc() {
        run.reset_get();
        resp.reset_put();
        pipe.reset();

        if (pipe.ready() && run.request()) {
            T data = run.get();
            pipe.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into PIPE " << data << endl;
        }
  
        if (pipe.request() && resp.ready()) {
            T data = pipe.get();
            resp.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from PIPE " << data << endl;
        }
    }
};

class simple_test : public sc_module 
{
public:
    using T = sct_uint<171>;

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
        a.run.clk_nrst(clk, nrst);
        a.resp.clk_nrst(clk, nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
        SCT_THREAD(init_thread);
        sensitive << run << resp << flip;
        async_reset_signal_is(nrst, 0);
    }
    
    sct_signal<bool> flip{"flip"};
    void init_thread()
    {
        flip = 0;
        run.reset_put();
        resp.reset_get();
        wait();

#ifdef __SC_TOOL__
        while (true) wait();
#else
        run.b_put(42); wait();
        run.b_put(43); wait();
        flip = !flip; wait();
        flip = !flip; wait();
        run.b_put(44); wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, put done " << endl;

        T data;
        data = resp.b_get(); wait();
        assert (data == 42);
        data = resp.b_get(); wait();
        assert (data == 43);
        data = resp.b_get(); wait();
        assert (data == 44);
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, done " << endl;

        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();    
#endif
    }
};

#endif /* SINGLE_TEST_H */

