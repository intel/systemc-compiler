/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Simple METHOD test with SYNC option in initiator
 */

#ifndef SIMPLE_METHOD_TEST_H
#define SIMPLE_METHOD_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

template<class T>
struct B : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run", 1};
    sct_initiator<T>    resp{"resp", 1};
    
    sct_in<T>           in{"in"};
    sct_signal<T>       ss{"ss"};
    sct_out<T>          out{"out"};
  
    SC_HAS_PROCESS(B);
    
    explicit B(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        SCT_METHOD(methProc);
        sensitive << run << resp << in;
    }
    
    sc_signal<T> s{"s"};
    void methProc() {
        ss = 0; 
        run.reset_get();
        resp.reset_put();
        
        if (run.request() && resp.ready()) {
            cout << sc_time_stamp() << " " << sc_delta_count()
                 << " : Get from target " << run.peek() << endl;
            resp.put(run.get());
            ss = run.peek();
        } else {
            cout << sc_time_stamp() << " " << sc_delta_count()
                 << " : NO get" << endl;
        }
        
        cout << "in " << in.read() << endl;
        out = in;
    }
    // Check SCT_ASSERT is disabled for TLM mode
    SCT_ASSERT(in.read(), SCT_TIME(0), out.read(), clk.pos());
};


class simple_test : public sc_module 
{
public:
    using T = sc_uint<16>;

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_initiator<T>    run{"run"};
    sct_target<T>       resp{"resp"};
    
    sct_signal<T>       in_s{"in_s"};
    sct_signal<T>       out_s{"out_s"};

    B<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "simple_method" << endl;
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
        a.in(in_s);
        a.out(out_s);
        
        SC_THREAD(init_thread);
        sensitive << run << resp;
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
    }
    
    const unsigned N = 4;
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        in_s = 0;
        wait();

        in_s = 42;
        while (!run.put(42)) wait();
        wait();
        while (!resp.get(data)) wait();
        assert (data == 42); wait();

        in_s = 43;
        while (!run.put(43)) wait();
        wait();
        in_s = 44;
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
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : all tests done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_METHOD_TEST_H */

