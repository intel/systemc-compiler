/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Clock and reset traits defined as compile definitions in CMakeLists.txt
 */

#ifndef CLK_RES_TRAITS_TEST_H
#define CLK_RES_TRAITS_TEST_H

#include "sct_common.h"
#include <systemc.h>

template<class T>
struct B : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run", 1};
    sct_initiator<T>    resp{"resp", 1};
    
    SC_HAS_PROCESS(B);
    
    explicit B(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        SC_METHOD(methProc);
        sensitive << run << resp;
    }
    
    sc_signal<T> s;
    void methProc() {
        run.reset_get();
        resp.reset_put();
        
        if (run.request() && resp.ready()) {
            cout << sc_time_stamp() << " " << sc_delta_count() 
                 << " : Get from target " << run.peek() << endl;
            resp.put(run.get());
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
    
    B<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "SCT_CMN_TLM_MODE " << SCT_CMN_TLM_MODE << endl;
        cout << "CLOCK " << SCT_CMN_TRAITS::CLOCK << endl;
        cout << "RESET " << SCT_CMN_TRAITS::RESET << endl;

        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
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
        wait();

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
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : all tests done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* CLK_RES_TRAITS_TEST_H */

