/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Multiple sequential puts/gets to target/initiator done from THREAD 
 * to check there no hangs up.
 */

#ifndef SIMPLE_THREAD_TEST_H
#define SIMPLE_THREAD_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

const unsigned N = 4;

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
        
        run.template add_fifo<N-1>();

        SC_THREAD(threadA);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    void threadA() {
        run.reset_get();
        resp.reset_put();
        wait();
        
        while(true) {
            // Waiting for N input requests
            unsigned i = 0;
            while (i < N && run.request()) {
                i++;
                wait();
            }
            cout << sc_time_stamp() << " " << sc_delta_count() 
                 << " : Run got all inputs " << i << endl;
            
            while (i && resp.ready()) {
                assert (run.request());
                T data = run.get();
                resp.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() 
                     << " : Resp " << data << endl;
                i--;
                wait();
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
        cout << "thread_multi_put_test" << endl;
                
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
        SC_THREAD(init_thread);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        wait();

        for (int i = 0; i < N; ++i) {
            run.b_put(42+i); 
            cout << sc_time_stamp() << " " << sc_delta_count() << " : TB put " << (42+i) << endl;
            wait();
        }
        
        for (int i = 0; i < N; ++i) {
            data = resp.b_get(); 
            cout << sc_time_stamp() << " " << sc_delta_count() << " : TB get " << data << endl;
            wait();
        }

        cout << sc_time_stamp() << " " << sc_delta_count() << " : all tests done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_THREAD_TEST_H */

