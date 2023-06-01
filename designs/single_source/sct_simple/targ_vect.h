/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Vector of target/initiator test
 */

#ifndef TARG_VECT_H
#define TARG_VECT_H

#include "sct_common.h"
#include <systemc.h>

template<class T, unsigned N>
struct B : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_vector<sct_target<T>> run{"run", N};
    sct_initiator<T>    resp{"resp"};
    
    SC_HAS_PROCESS(B);
    
    explicit B(const sc_module_name& name) : sc_module(name)
    {
        for (unsigned i = 0; i < N; ++i) run[i].clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        SC_METHOD(methProc);
        for (unsigned i = 0; i < N; ++i) sensitive << run[i];
        sensitive << resp << s;
    }
    
    sc_signal<T> s{"s"};
    
    void methProc() {
        for (unsigned i = 0; i < N; ++i) run[i].reset_get();
        resp.reset_put();
        unsigned i = s.read();
        
        for (int i = 0; i < N; ++i) {
            bool a = run[i].request();
        }
        
        //resp.put(run[0].get());
    }
};


class simple_test : public sc_module 
{
public:
    using T = sc_uint<16>;
    static const unsigned N = 10;

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_vector<sct_initiator<T>>  run{"run", N};
    sct_target<T>       resp{"resp"};
    
    B<T, N>      a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "simple_method" << endl;

        for (unsigned i = 0; i < N; ++i) run[i].clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);

        for (unsigned i = 0; i < N; ++i) a.run[i].bind(run[i]);
        a.resp.bind(resp);
        
        SC_THREAD(init_thread);
        for (unsigned i = 0; i < N; ++i) sensitive << run[i];
        sensitive << resp;
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
    }
    
    void init_thread()
    {
        T data; 
        for (unsigned i = 0; i < N; ++i) run[i].reset_put();
        resp.reset_get();
        wait();

        while (!run[0].put(42)) wait();
        wait();
        while (!resp.get(data)) wait();
        assert (data == 42); wait();

        cout << sc_time_stamp() << " " << sc_delta_count() << " : all tests done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};


#endif /* TARG_VECT_H */

